#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include "../base/noncopyable.h"
#include "../base/Thread.h"
#include "EventLoop.h"

#include <functional>
#include <mutex>
#include <condition_variable>

class EventLoop;
//在一个线程中运行一个EventLoop
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    //参数：线程初始化回调， 线程名称
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());

    ~EventLoopThread();

    //关键函数，调用threadFunc,等待EventLoop创建完成
    EventLoop* startLoop();

private:
    //新线程的主函数，用于在其内部创建EventLoop
    void threadFunc();

    EventLoop* loop_;  //EventLoop对象
    bool exiting_;  //退出标志
    Thread thread_;  //线程类
    std::mutex mutex_;  //互斥锁
    std::condition_variable cond_;  //条件变量，通知startLoopEventLoop已创建完
    ThreadInitCallback callback_;  //用于初始化EventLoop的内容
};

#endif