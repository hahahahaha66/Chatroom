#ifndef POLLER_H
#define POLLER_H

#include "../../base/noncopyable.h"
#include "../Channel.h"
#include "../../base/Timestamp.h"

#include <vector>
#include <unordered_map>

//抽象版本，提供大致的接口
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    //交给派生类实现的接口
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    //判断channel是否已经注册到Poller中
    bool hasChannel(Channel* channel) const;

    //EventLoop通过该接口获取默认的IO复用实现方式（默认epoll）
    static Poller* newDefaultPoller(EventLoop* Loop);

protected:
    //存储channel的映射，通过sockfd->channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;  //已经注册的channel

private:
    EventLoop* ownerLoop_;  //定义Poller事件所属EventLoop
};

#endif 