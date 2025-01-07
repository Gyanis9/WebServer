//
// Created by guo on 25-1-6.
//

#include <fcntl.h>

#include "../include/Acceptor.h"
#include "../include/EventLoop.h"
#include "../include/InetAddress.h"
#include "../../base/include/Logger.h"

static int createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL << "create sockfdfail";
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool requesport) : m_loop(loop),
                                                                                      m_acceptSocket(
                                                                                              createNonblocking()),
                                                                                      m_acceptChannel(loop,m_acceptSocket.fd()),
                                                                                      m_listening(false) {
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(true);
    m_acceptSocket.bindAddress(listenAddr);
    m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}

void Acceptor::setNewConnectionCallback(const Acceptor::NewConnectionCallback &cb) {
    m_newConnectionCallback = std::move(cb);
}

void Acceptor::listen() {
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}

void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);
    if (connfd >= 0) {
        if (m_newConnectionCallback != nullptr) {
            m_newConnectionCallback(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_SYSERR << "in Acceptor::handleRead";
        if (errno == EMFILE) {
            LOG_ERROR << "sockfd reached limit";
        }
    }
}
