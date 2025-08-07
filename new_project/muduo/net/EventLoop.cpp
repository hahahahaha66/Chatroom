#include "EventLoop.h"
#include "../logging/Logging.h"
#include "Channel.h"
#include "poller/Poller.h"

#include <cstdint>
#include <fcntl.h>
#include <mutex>
#include <sys/eventfd.h>
#include <unistd.h>

__thread EventLoop *t_loopInthisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd() {
    // 创建一个轻量级的文件描述符，通过向该文件描述符写，来唤醒IO线程
    // 主要用于跨线程唤醒
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        LOG_FATAL << "eventfd error: " << errno;
    }
    return evfd;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false), callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)), wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr) {
    LOG_DEBUG << "EventLoop created " << this << " this index is " << threadId_;
    LOG_DEBUG << "EventLoop created wakeupFd" << wakeupChannel_->fd();
    if (t_loopInthisThread) {
        LOG_FATAL << "Another EventLoop" << t_loopInthisThread
                  << " exists in this thread " << threadId_;
    } else {
        t_loopInthisThread = this;
    }

    // 将自身包装成Channel,写成可读事件，用于唤醒
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInthisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true; // 在事件循环中
    quit_ = false;   // 不处于退出状态

    LOG_INFO << "EventLoop " << this << " start looping";

    while (!quit_) // 循环接收任务
    {
        activeChannels_.clear();
        // 获取活跃的channel
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        // 循环处理channel
        for (Channel *channel : activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        // 在当前循环中处理上一次循环的任务
        doPendingFunctors();
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;

    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) // 是EventLoop当前线程的任务，立即执行
    {
        cb();
    } else // 否则加入任务队列
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_); // 上锁，防止竞态
        pendingFunctors_.emplace_back(cb);         // 对共用的任务队列提交任务
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    // 向wakeupFd_写入一个8字节的整数，会让epoll监视变为可读事件
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    // 相应可读事件，读取8字节的整数
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n
                  << " bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    // 循环处理任务队列中的任务
    for (const Functor &functor : functors) {
        if (functor) {
            functor();
        }
    }

    callingPendingFunctors_ = false;
}