#include "Channel.h"
#include "EventLoop.h"

#include <memory>
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
        : loop_(loop), 
        fd_(fd), 
        events_(0), 
        revents_(0),
        index_(-1),
        tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

//受到Poller通知后，检查tied,并调用handleEventWithGuard
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        //这里将weak_ptr升级为share_ptr确保在回调函数运行期间连接一直存在
        //重点注意这里的升权操作
        std::shared_ptr<void> guard = tie_.lock();
        //如果guard为空，说明Channel的TcpConnection对象已经不在了
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else 
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    //对端关闭事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    //错误事件
    if (revents_ & EPOLLERR)
    {
        LOG_DEBUG << "the fd = " << this->fd();
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    //读事件
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        LOG_DEBUG << "channel have read events, the fd =" << this->fd();
        if (readCallback_)
        {
            LOG_DEBUG << "channel call the readCallback_(), the fd = " << this->fd();
            readCallback_(receiveTime);
        }
    }

    //写事件
    if (revents_ & EPOLLOUT)
    {
        
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}
