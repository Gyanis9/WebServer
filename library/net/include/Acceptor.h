//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_ACCEPTOR_H
#define WEBSERVER_ACCEPTOR_H

#include "../../base/include/noncopyable.h"
#include "Channel.h"
#include "Socket.h"

class EventLoop;

class InetAddress;

class Acceptor : private noncopyable {
public:
    using NewConnectionCallback = std::function<void(int fd, const InetAddress &)>;
public:
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool requesport);

    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb);

    void listen();

    bool listening() const { return m_listening; }


private:
    void handleRead();

private:
    EventLoop *m_loop;
    Socket m_acceptSocket;
    Channel m_acceptChannel;
    bool m_listening;

    NewConnectionCallback m_newConnectionCallback;
};

static int createNonblocking();

#endif //WEBSERVER_ACCEPTOR_H
