//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_TCPCONNECTION_H
#define WEBSERVER_TCPCONNECTION_H

#include <memory>

#include "../../base/include/noncopyable.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"

class Channel;

class EventLoop;

class Socket;


class TcpConnection : private noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                  const InetAddress &peerAddr);

    ~TcpConnection();

    EventLoop *getLoop() const { return m_loop; }

    const std::string &name() const { return m_name; }

    const InetAddress &localAddress() const { return m_localAddr; }

    const InetAddress &peerAddress() const { return m_peerAddr; }

    bool connected() const { return m_state == kConnected; }

    bool disConnected() const { return m_state == kDisconnected; }


    void send(const std::string &buf);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = std::move(cb); }

    void setMessageCallback(const MessageCallback &cb) { m_messageCallback = std::move(cb); }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = std::move(cb); }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { m_highWaterMarkCallback = std::move(cb); }

    void setCloseCallback(const CloseCallback &cb) { m_closeCallback = std::move(cb); }

    void connectEstablished();

    void connectDestroyed();

private:
    enum StateE {
        kDisconnected, kConnecting, kConnected, kDisconnecting
    };

    void handleRead(Timestamp receiveTime);

    void handleWrite();

    void handleClose();

    void handleError();

    void sendInLoop(const void *data, size_t len);

    void shutdownInLoop();

    void setState(StateE s) { m_state = s; }

private:
    EventLoop *m_loop;
    const std::string m_name;
    StateE m_state;
    bool m_reading;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    CloseCallback m_closeCallback;

    size_t m_highWaterMark;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
};

EventLoop *CheckLoopNotNull(EventLoop *loop);

#endif //WEBSERVER_TCPCONNECTION_H
