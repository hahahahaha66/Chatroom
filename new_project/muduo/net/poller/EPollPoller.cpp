#include "EPollPoller.h"

#include <cstddef>
#include <string.h>
#include <sys/epoll.h>

//使用以下数字简单代指channel的状态
const int kNew = -1;  //从未添加过
const int kAdded = 1;  //已经添加
const int kDeleted = 2;  //曾经添加但现在已被删除

EPollPoller::EPollPoller(EventLoop* loop) 
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
    if (epollfd_ < 0) 
    {
        LOG_FATAL << "epoll_creats() error : " << errno;
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &(*events_.begin()),
                static_cast<int>(events_.size()), timeoutMs);
    
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
        
        //扩充events_的空间大小
        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    //超时
    else if (numEvents == 0)
    {
        LOG_DEBUG << "timeout !" ;
    }
    //出错
    else 
    {
        //防止由于是终端发送类似SIGINT信号造成的错误
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR << "EPollPoller::poll() failed";
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();

    //未添加
    if (index == kNew || index == kDeleted)
    {   
        //从未添加
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        else 
        {
        }
        
        //修改channel的index
        channel->set_index(kAdded);
        //增加事件
        update(EPOLL_CTL_ADD, channel);
    }
    //已添加
    else 
    {   
        //如果没有事件就删除
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else 
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

//添加活跃事件
void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    //如果已被添加，则删除
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    //更新对应的index
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    //提取对应channel的信息
    int fd = channel->fd();
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    //处理ADD,MOD，DEL等操作
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR << " epoll_ctl() del error: " << errno << " (" << strerror(errno) << ")";
        }
        else 
        {
            LOG_FATAL << " epoll_ctl add/mod error: " << errno << " (" << strerror(errno) << ")";
        }
    }
}
