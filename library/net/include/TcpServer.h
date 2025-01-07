//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_TCPSERVER_H
#define WEBSERVER_TCPSERVER_H

#include "TcpConnection.h"
#include "../../base/include/noncopyable.h"


#include <unordered_map>
#include <atomic>

class Acceptor;

class EventLoop;

class EventLoopThreadPool;

class TcpServer : private noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option {
        kNoReusePort,
        kReusePort,
    };
public:
    TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option = kNoReusePort);

    ~TcpServer();

    const std::string &ipPort() const { return m_ipPort; }

    const std::string &name() const { return m_name; }

    EventLoop *getLoop() const { return m_loop; }

    void setThreadNum(int numThreads);

    std::shared_ptr<EventLoopThreadPool> threadPool() { return m_threadPool; }

    void start();

    void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = std::move(cb); }

    void setMessageCallback(const MessageCallback &cb) { m_messageCallback = std::move(cb); }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = std::move(cb); }

    void setThreadInitCallback(const ThreadInitCallback &cb) { m_threadInitCallback = std::move(cb); }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);

    void removeConnection(const TcpConnectionPtr &conn);

    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *m_loop;
    const std::string m_ipPort;
    const std::string m_name;
    std::unique_ptr<Acceptor> m_acceptor;
    std::shared_ptr<EventLoopThreadPool> m_threadPool;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    ThreadInitCallback m_threadInitCallback;
    std::atomic<int32_t> m_started;
    int m_nextConnID;
    ConnectionMap m_connections;
};


#endif //WEBSERVER_TCPSERVER_H
