//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H

#include "../../base/include/noncopyable.h"

class InetAddress;

class Socket : private noncopyable {
public:
    explicit Socket(const int sockfd);

    ~Socket();

    int fd() const { return m_sockfd; }

    void bindAddress(const InetAddress &localaddr);

    void listen();

    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

private:
    const int m_sockfd;
};

#endif //WEBSERVER_SOCKET_H
