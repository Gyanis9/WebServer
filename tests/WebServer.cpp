//
// Created by guo on 25-1-2.
//

#include "../library/net/include/TcpServer.h"
#include "../library/net/include/EventLoop.h"
#include "../library/base/include/Logger.h"
//#include <muduo/net/TcpServer.h>
//#include <muduo/net/InetAddress.h>
//#include <muduo/net/EventLoop.h>
//#include <muduo/base/Logging.h>


#include <string>
#include <functional>
#include <iostream>

//using namespace muduo;
//using namespace muduo::net;

class EchoServer {
public:
    EchoServer(EventLoop *loop,
               const InetAddress &addr,
               const std::string &name)
            : m_loop(loop), m_server(loop, addr, name) {

        m_server.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );

        m_server.setMessageCallback(
                std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3)
        );

        m_server.setThreadNum(8);
    }

    ~EchoServer() = default;

    void start() {
        m_server.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn) {
        if (conn->connected()) {
            LOG_INFO << "Connection UP : " << conn->peerAddress().toIpPort().c_str();
        } else {
            LOG_INFO << "Connection DOWN : " << conn->peerAddress().toIpPort().c_str();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
        std::string request = buf->retrieveAllAsString();
        std::cout << "Received request:\n" << request << std::endl; // 调试输出

        // 构建 HTTP 响应
        std::string response;
        response += "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 13\r\n";
        response += "Connection: close\r\n"; // 可选，根据需要决定是否保持连接
        response += "\r\n";
        response += "Hello, world!";

        // 发送响应
        conn->send(response);

        // 关闭连接（如果不需要保持连接）
        conn->shutdown();
    }

private:
    TcpServer m_server;
    EventLoop *m_loop;
};

int main() {
//    Logger::setLogLevel(Logger::LogLevel::TRACE);
    EventLoop loop;
    InetAddress addr(8080);
    EchoServer server(&loop, addr, "EchoServer-01");
    server.start();
    loop.loop();

    return 0;
}

