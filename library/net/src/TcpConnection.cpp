//
// Created by guo on 25-1-6.
//

#include "../include/TcpConnection.h"
#include "../../base/include/Logger.h"
#include "../include/Socket.h"
#include "../include/Channel.h"
#include "../include/EventLoop.h"

EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL << "loop is nullptr ";
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr) : m_loop(CheckLoopNotNull(loop)),
                                                            m_name(name),
                                                            m_state(kConnecting),
                                                            m_reading(true),
                                                            m_socket(new Socket(sockfd)),
                                                            m_channel(new Channel(loop, sockfd)),
                                                            m_localAddr(localAddr),
                                                            m_peerAddr(peerAddr),
                                                            m_highWaterMark(64 * 1024 * 1024) {
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" << m_name << "] at " << this << " fd " << sockfd;
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
              << " fd=" << m_channel->fd();
    assert(m_state == kDisconnected);
}


void TcpConnection::send(const std::string &buf) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}


void TcpConnection::connectEstablished() {
    assert(m_state == kConnecting);
    setState(kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();
        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int saveError = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveError);
    if (n > 0) {
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveError;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (m_channel->isWriting()) {
        int saveErrno = 0;
        ssize_t n = m_outputBuffer.writeFd(m_channel->fd(), &saveErrno);
        if (n > 0) {
            m_outputBuffer.rretrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                m_channel->disableWirting();
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << "Connection fd = " << m_channel->fd() << " is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    assert(m_state == kConnected || m_state == kDisconnecting);
    setState(kDisconnected);
    m_channel->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    m_connectionCallback(guardThis);
    m_closeCallback(guardThis);
}

void TcpConnection::handleError() {
    int optval = 0;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(m_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR << "TcpConnection::handleError name:" << m_name.c_str() << " - SO_ERROR:" << err;
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (m_state == kDisconnected) {
        LOG_ERROR << "disconnected,give up writing errno:" << errno;
        return;
    }
    if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) {
        nwrote = ::write(m_channel->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = m_outputBuffer.readableBytes();
        if (oldLen + remaining >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
        }
        m_outputBuffer.append(static_cast<const char *>(data) + nwrote, remaining);
        if (!m_channel->isWriting()) {
            m_channel->enableWriting();
        }

    }
}

void TcpConnection::shutdownInLoop() {
    if (!m_channel->isWriting()) {
        m_socket->shutdownWrite();
    }
}



