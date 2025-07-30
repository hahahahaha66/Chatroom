#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include "../../logging/Logging.h"
#include "../EventLoop.h"
#include "Poller.h"
#include "../../base/Timestamp.h"

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

//Poller的具体实现版本，使用epoll作为事件分发机制
class EPollPoller : public Poller
{
    using EventList = std::vector<epoll_event>;

public:
    //构造，绑定EventLoop
    EPollPoller(EventLoop* Loop);
    ~EPollPoller() override;

    //重写基类的虚函数
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;  //等待事件发生
    void updateChannel(Channel* channel) override;  //添加或更新事件
    void removeChannel(Channel* channel) override;  //移除channel

private:
    static const int kInitEventListSize = 16;

    //填充活跃的channel
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    //处理具体的事件操作，如ADD,MOD,DEL
    void update(int operation, Channel* channel);

    int epollfd_;  //epoll的文件描述符
    EventList events_;  //管理的channel列表
};

#endif