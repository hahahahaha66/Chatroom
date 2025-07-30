#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "../base/Timestamp.h"
#include "../net/Channel.h"

#include <functional>
#include <sys/timerfd.h>
#include <utility>
#include <vector>
#include <set>

class EventLoop;
class Timer;

//创建一个timefd,并绑定到一个channel,将channel设为epoll感兴趣的事件
//当定时器到期时，对应文件描述变为可读或可写，EventLoop调用回调函数处理超时事件
class TimerQueue
{
public:
    using TimerCallback = std::function<void()>;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    //插入定时器，包括回调函数，到期时间，是否重复
    void addTimer(TimerCallback cb, Timestamp when, double interval);

private:
    using Entry = std::pair<Timestamp, Timer*>;  //使用时间戳作为键值索引定时器
    using TimerList = std::set<Entry>;  //按照时间戳自动排序

    void addTimerInLoop(Timer* timer);   //添加定时器

    void handlleRead();  //定时器读事件触发的函数

    void resetTimerfd(int timerfd_, Timestamp expiration);  //重新设置超时时间

    //移除超时的定时器
    std::vector<Entry> getExpired(Timestamp now);
    
    //重新设置定时器的时间
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer_);  //向TimerList中插入一个新的定时器，返回是否是新的最早的定时器

    EventLoop* loop_;  //所属的EventLoop
    const int timerfd_;  //linux的一个内核定时器机制，可以对它设置超时
    Channel timerfdChannel_;  //封装time_fd，时间到了，timerfdChannel_回调被触发

    TimerList timers_;  //使用set实现定时器队列，储存所有的定时器

    bool callingExpiredTimers_;  //是否正在处理回调，防止重入
};

#endif