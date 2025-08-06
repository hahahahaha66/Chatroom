#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip) {
    ::bzero(&addr_, sizeof(addr_)); // 内存清零
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port); // 端口主机转网络
    // 监听所有的ip地址，也就是0.0.0.0
    // addr_.sin_addr.s_addr = htonl(INADDR_ANY);  //IP主机转网络
    ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf,
                sizeof(buf)); // 二进制addr转字符串IP
    return buf;
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf,
                sizeof(buf)); // 二进制addr转字符串IP
    size_t end = ::strlen(buf);
    uint16_t port = ntohs(addr_.sin_port); // 端口网络转主机
    sprintf(buf + end, ":%u", port);       // 格式 0.0.0.0:8080
    return buf;
}

uint16_t InetAddress::toPort() const {
    return ::ntohs(addr_.sin_port); // 端口网络转主机
}
