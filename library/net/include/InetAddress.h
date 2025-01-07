//
// Created by guo on 25-1-6.
//

#ifndef WEBSERVER_INETADDRESS_H
#define WEBSERVER_INETADDRESS_H

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include "../../base/include/copyable.h"

class InetAddress : public copyable {
public:
    explicit InetAddress(uint16_t port = 0, const std::string &ip = "127.0.0.1");

    explicit InetAddress(const sockaddr_in &addr) : m_addr(addr) {}

    /**获取IP地址*/
    std::string toIp() const;

    /**获取IP+端口号*/
    std::string toIpPort() const;

    /**获取端口号*/
    uint16_t toPort() const;

    /**获取IP协议信息*/
    const sockaddr_in *getSockAddr() const { return &m_addr; }

    void setSockAddr(const sockaddr_in &addr) { m_addr = addr; }

private:
    sockaddr_in m_addr{};
};


#endif //WEBSERVER_INETADDRESS_H
