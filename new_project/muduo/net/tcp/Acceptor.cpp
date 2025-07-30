#include "Acceptor.h"
#include "../../logging/Logging.h"

static int createNonblocking()
{
    //获取监听套接字
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL << "Listen socket create errno " << errno;
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& ListenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(createNonblocking()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false)
{
    LOG_DEBUG << "Acceptor create noblocking socket, [fd = " << acceptChannel_.fd() << "]";
    acceptSocket_.setReuseAddr(reuseport);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(ListenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    //取消所有的感兴趣的事件
    acceptChannel_.disableAll();
    //删除该事件
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}
    
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd, peerAddr);
        }
        else 
        {
            LOG_DEBUG << "no newConnectionCallback() function";
            ::close(connfd);
        }
    }
    else 
    {
        LOG_ERROR << "accept() failed";
        if (errno == EMFILE)
        {
            LOG_ERROR << "sockfd reached limited";
        }
    }
}

