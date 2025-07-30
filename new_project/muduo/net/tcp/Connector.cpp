#include "Connector.h"
#include "../../logging/Logging.h"
#include "../Channel.h"
#include "../EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"

#include <errno.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    if (connect_)
    {
        connect();
    }
    else 
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop,this));
}

void Connector::stopInLoop()
{
    if (state_ == kConnecting)
    {
        setStates(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    int ret = ::connect(sockfd, (sockaddr*)serverAddr_.getSockAddr(), sizeof(serverAddr_));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:  //连接成功
    case EINPROGRESS:  //正在连接
    case EINTR:  //被系统信号中断
    case EISCONN:  //连接已建立
        connecting(sockfd);
        break;

    case EAGAIN:  //资源暂不可用
    case EADDRINUSE:  //地址正在使用
    case EADDRNOTAVAIL:  //地址不可用
    case ECONNREFUSED:  //连接被拒绝
    case ENETUNREACH:  //网络不可达
        retry(sockfd);
        break;

    case EACCES:  //权限不足
    case EPERM:  //操作不允许
    case EAFNOSUPPORT:  //地址族不支持
    case EALREADY:  //操作已在进行中
    case EBADF:  //文件描述符无效
    case EFAULT:  //无效地址
    case ENOTSOCK:  //文件描述符不是一个套接字
        LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
        ::close(sockfd);
        break;

    default:
        LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
        ::close(sockfd);
        // connectErrorCallback_();
        break;
  }
}

void Connector::restart()
{
    setStates(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd)
{
    setStates(kConnecting);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));
    channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    channel_.reset();
}

//获取套接字的本地地址
struct sockaddr_in Connector::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (sockaddr*)(&localaddr), &addrlen) < 0)
    {
        LOG_ERROR << "sockets::getLocalAddr";
    }
    return localaddr;
}


//获取套接字的对端地址
struct sockaddr_in Connector::getPeerAddr(int sockfd)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, (sockaddr*)(&peeraddr), &addrlen) < 0)
    {
        LOG_ERROR << "sockets::getPeerAddr";
    }
    return peeraddr;
}


//用于检查是否发生了自连接
bool Connector::isSelfConnect(int sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);
    
    // 判断是否是自连接（IPV4）
    return localaddr.sin_port == peeraddr.sin_port && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

//获取套接字的错误状态
int Connector::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else 
    {
        return optval;
    }
}

void Connector::handleWrite()
{
    LOG_INFO << "Connector::handlewriet " << state_;

    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        
        if (err)
        {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " " << strerror(err);
            retry(sockfd);
        }
        else if (isSelfConnect(sockfd))
        {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setStates(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                ::close(sockfd);
            }
        }
    }
    // else
    // {
    //     // what happened?
    //     assert(state_ == kDisconnected);
    // }
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError state=" << state_;
    if (state_ == kConnecting)
    {
        int socket = removeAndResetChannel();
        int err = getSocketError(socket);
        LOG_ERROR << "SO_ERROR = " <<  err << " " << strerror(err);
        retry(socket);
    }
}

void Connector::retry(int sockfd)
{
    ::close(sockfd);
    setStates(kDisconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::retry - Retry connecting to" << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_ << " milliseconds.  ";
        loop_->runAfter(retryDelayMs_/1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);  //获取重试时间
    }
    else  
    {
        LOG_DEBUG << "do not connect";
    }
}