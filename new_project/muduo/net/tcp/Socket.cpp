#include "Socket.h"
#include "../../logging//Logging.h"
#include "InetAddress.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    if (::bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)) != 0)
    {
        perror("bind error");
        LOG_FATAL<< "bind sockfd: " << sockfd_;
    }
}

void Socket::listen()
{
    //最大允许1024个同时连接
    if (::listen(sockfd_, 1024) != 0)
    {
        LOG_FATAL << " listen sockfd: " << sockfd_ << " failed";
    }
}

int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::memset(&addr, 0, sizeof(addr));

    //设置非阻塞和关闭执行继承，并且accept4是原子操作
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        //保存新连接的sockaddr
        peeraddr->setSockAddr(addr);
    }
    else 
    {
        LOG_ERROR << " accept4() failed";
    }
    //返回新连接的文件描述符
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR << " shutdownWrite error";
    }
}

//Nagel算法是为了解决"发送大量小包"导致的网络拥塞问题。
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

//设置后就可以实现多线程共享监听接口
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
