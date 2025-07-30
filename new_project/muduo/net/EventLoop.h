#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include "../base/Timestamp.h"
#include "../timer/TimerQueue.h"
#include "Channel.h"

#include <ctime>
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

class Channel;
class Poller;

//事件循环类，主要分为channel和poller模块
class EventLoop 
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //真正的核心函数
    //负责通过（Poller）监听事件，触发事件处理（Channel），执行待办任务
    void loop();

    void quit();

    Timestamp pollReturnTimer() const { return pollReturnTime_; }

    //当前线程调用则立即执行任务
    void runInLoop(Functor cb);

    //提交到任务队列，稍后处理
    void queueInLoop(Functor cb);

    //唤醒阻塞的EventLoop::loop
    void wakeup();

    //更新channel
    void updateChannel(Channel* channel);
    //移除channel
    void removeChannel(Channel* channel);
    //判断是否有channel
    bool hasChannel(Channel* channel);

    //判断当前调用这个函数的线程是否是当前EventLoop线程
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); };

    //定时器任务
    //正常定时任务
    void runAt(Timestamp timestamp, Functor&& cb)
    {
        timerQueue_->addTimer(std::move(cb), timestamp, 0.0);
    }

    //延时定时任务
    void runAfter(double waitTime, Functor&& cb)
    {
        Timestamp time(addTime(Timestamp::now(), waitTime));
        runAt(time, std::move(cb));
    }

    //循环定时任务
    void runEvery(double interval, Functor&& cb)
    {
        Timestamp timestamp(addTime(Timestamp::now(), interval));
        timerQueue_->addTimer(cb, timestamp, interval);
    }

private:
    //响应唤醒事件，清除唤醒状态
    void handleRead();

    //执行任务队列中的任务
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    //原子操作
    std::atomic_bool looping_;  //是否在事件循环中
    std::atomic_bool quit_;  //是否退出事件循环
    std::atomic_bool callingPendingFunctors_;  //是否正在执行回调操作

    const pid_t threadId_;  //EventLoop当前线程id，一个EventLoop绑定唯一一个线程id

    Timestamp pollReturnTime_;  //poller返回发生事件的channel的返回时间

    std::unique_ptr<Poller> poller_;  //绑定poller（封装了epoll_wait）
    std::unique_ptr<TimerQueue> timerQueue_;  //管理定时器事件

    int wakeupFd_;  //存储eventfd,用于线程间的事件通知
    std::unique_ptr<Channel> wakeupChannel_;  //绑定wakeupFd_，封装成Channel,能加入到epoll

    ChannelList activeChannels_;  //活跃的channel
    Channel* currentActiveChannel_;  //正在处理的channel

    std::mutex mutex_;  //线程锁
    std::vector<Functor> pendingFunctors_;  //保存待执行的任务
};

#endif