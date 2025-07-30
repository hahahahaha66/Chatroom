#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <string.h>

class InetAddress
{
public:
    //未初始化默认ip为127.0.0.1
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    
    explicit InetAddress(const sockaddr_in& addr)
        :addr_(addr)
    {
    }

    std::string toIp() const;  //获取IP
    std::string toIpPort() const;  //获取IP和端口
    uint16_t toPort() const;  //获取端口

    const sockaddr_in* getSockAddr() const { return &addr_; }
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }

private:
    sockaddr_in addr_;  //网络地址结构体，包括地址族，端口，ip
};

#endif