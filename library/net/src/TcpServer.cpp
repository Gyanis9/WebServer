//
// Created by guo on 25-1-6.
//
#include "../../base/include/Logger.h"
#include "../include/TcpServer.h"
#include "../include/Acceptor.h"
#include "../include/EventLoopThreadPool.h"
#include "../include/EventLoop.h"

#include <cstring>


TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg,
                     TcpServer::Option option) : m_loop(CheckLoopNotNull(loop)),
                                                 m_ipPort(listenAddr.toIpPort()),
                                                 m_name(nameArg),
                                                 m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
                                                 m_threadPool(new EventLoopThreadPool(loop, m_name)),
                                                 m_connectionCallback(),
                                                 m_messageCallback(),
                                                 m_nextConnID(1), m_started(0) {
    m_acceptor->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    LOG_TRACE << "TcpServer::~TcpServer [" << m_name << "] destructing";
    for (auto &item: m_connections) {
        auto conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (m_started++ == 0) {
        m_threadPool->start(m_threadInitCallback);
        assert(!m_acceptor->listening());
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
        LOG_DEBUG << "Server started and listening on " << m_ipPort;
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = m_threadPool->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s%d", m_ipPort.c_str(), m_nextConnID);
    ++m_nextConnID;
    std::string connName = m_name + buf;
    LOG_INFO << "TcpServer::newConnection [" << m_name << "] - new connection [" << connName << "] from "
             << peerAddr.toIpPort();
    sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addlen = static_cast<socklen_t >(sizeof localaddr);
    if (::getsockname(sockfd, (sockaddr *) &localaddr, &addlen) < 0) {
        LOG_ERROR << "sockets::getLocalAddr errno" << errno;
    }
    InetAddress localAddr(localaddr);
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    m_connections[connName] = conn;
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << m_name << " ] - connection" << conn->name();
    m_connections.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
