//
// Created by guo on 25-1-6.
//

#include "../include/Socket.h"
#include "../include/InetAddress.h"
#include "../../base/include/Logger.h"
#include <sys/socket.h>
#include <cstring>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

Socket::Socket(const int sockfd) : m_sockfd(sockfd) {

}

Socket::~Socket() {
    ::close(m_sockfd);
}

void Socket::bindAddress(const InetAddress &localaddr) {
    if (::bind(m_sockfd, (sockaddr *) localaddr.getSockAddr(), sizeof(sockaddr_in)) != 0) {
        LOG_FATAL << "bing sockfd fail";
    }
}

void Socket::listen() {
    if ((::listen(m_sockfd, SOMAXCONN)) < 0) {
        LOG_FATAL << "listen scokfd fail";
    }
}

int Socket::accept(InetAddress *peeraddr) {
    sockaddr_in addr;
    bzero(&addr, sizeof addr);
    socklen_t len = sizeof(addr);
    int connfd = ::accept4(m_sockfd, (sockaddr *) &addr, &len, SOCK_NONBLOCK);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(m_sockfd, SHUT_WR) < 0) {
        LOG_ERROR << "sockets:shuwdownWrite error errno:%d" << errno;
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}


