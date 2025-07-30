#ifndef SOCKET_H
#define SOCKET_H

#include "../../base/noncopyable.h"
#include "InetAddress.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

class InetAddress;

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd )
        : sockfd_(sockfd)
    {
    }

    ~Socket();

    //获取sockfd
    int fd() const { return sockfd_; };

    //绑定地址
    void bindAddress(const InetAddress& localdaddr); 

    //监听
    void listen();  

    //连接，perraddr保存新连接的地址，返回新连接的套接字
    int accept(InetAddress* perraddr);  

    //关闭写端
    void shutdownWrite();

    //关闭/开启Nagle算法
    void setTcpNoDelay(bool on);  
    //服务器重启时可以立即重新绑定端口
    void setReuseAddr(bool on); 
    //允许多个socket同时绑定到同一个IP+端口
    void setReusePort(bool on);  
    //定期检测连接是否可用
    void setKeepAlive(bool on); 

private:
    const int sockfd_;  //sock套接字
};

#endif