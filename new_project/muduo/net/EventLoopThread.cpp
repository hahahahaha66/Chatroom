#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(nullptr), exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name), mutex_(),
      cond_(), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    // 启动线程
    thread_.start();

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待loop_创建完成
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    // 在EventLoop创建前调用，提供所需的变量
    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); // 事件循环

    // 此时事件循环已经结束，需要关闭loop_,但同时其他线程可能还持有loop_所以需要上锁
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
