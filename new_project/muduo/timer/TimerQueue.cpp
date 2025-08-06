#include "TimerQueue.h"
#include "../base/Timestamp.h"
#include "../logging/Logging.h"
#include "../net/EventLoop.h"
#include "Timer.h"

// 生成Linux内核的定时器
int createTimerfd() {
    // CLOCK_MONOIONIC 使用单调递增的时钟（不受系统时间变化影响）
    // TFD_NONBBLOCK 非阻塞读
    // TFD_CLOEXEC   执行exec调用时关闭fd

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0) // 创建失败
    {
        LOG_ERROR << "Failed in timerfd_create";
    }
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(createTimerfd()), timerfdChannel_(loop_, timerfd_),
      timers_() {
    // 使用bind将读事件打包
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handlleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);

    // 删除所有的定时器
    for (const Entry &timer : timers_) {
        delete timer.second;
    }
}

// 添加定时器：cd 回调函数 when 第一次触发时间 interval 是否是周期定时
void TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval) {
    Timer *timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

// 定时器插入数据结构中
void TimerQueue::addTimerInLoop(Timer *timer) {
    // 由于Linux内核定时器只有一个超时时间
    // 如果当前插入的定时器的时间点早于当前最早的定时器的时间点
    // 就要重新设置timerfd的超时时间

    bool eraliestChanged = insert(timer); // 是否取代了最早的定时触发时间

    // 需要重新设置timerfd_触发时间
    if (eraliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::resetTimerfd(int timerfd_, Timestamp expiration) {
    // timerspec是timerfd_settime使用的结构体
    // 用于描述定时器首次触发，之后是否周期性触发
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, '\0', sizeof(newValue));
    memset(&oldValue, '\0', sizeof(oldValue));

    // 获取新的触发时间
    int64_t microSecondDif = expiration.microSecondsSinceEpoch() -
                             Timestamp::now().microSecondsSinceEpoch();
    // 至少设为100微秒，防止立即触发（防止时间精度不足导致立即触发）
    if (microSecondDif < 100) {
        microSecondDif = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSecondDif /
                                    Timestamp::kMicroSecondsPerSecond); // 秒
    ts.tv_nsec = static_cast<long>(
        (microSecondDif % Timestamp::kMicroSecondsPerSecond) * 1000); // 微秒
    newValue.it_value = ts; // 设置第一次触发时间

    if (::timerfd_settime(timerfd_, 0, &newValue, &oldValue)) {
        LOG_ERROR << "timerfd_settime failed()";
    }
}

// 定时器超时发生后，epoll通知，此时timerfd变为可读
// 但如果不read，它会一直处于可读事件，epoll不再重复通知
// 所以需要立即读取（消费）这个可读事件，让epoll能再次触发
void ReadTimerFd(int timerfd) {
    uint64_t read_byte;
    ssize_t readn = ::read(timerfd, &read_byte, sizeof(read_byte));

    if (readn != sizeof(read_byte)) {
        LOG_ERROR << "TimerQueue::ReadTimerFd read_size < 0";
    }
}

// 获取当前时间已经超时的所有定时器
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;

    // 设置当前时间
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    // 找到定时器中第一个大于等于now的定时器
    TimerList::iterator end = timers_.lower_bound(sentry);
    // 将所有的超时的定时器拷贝出来
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    // 从set中删除所有的过期定时器
    timers_.erase(timers_.begin(), end);

    return expired;
}

// 处理到期定时器
void TimerQueue::handlleRead() {
    Timestamp now = Timestamp::now();
    ReadTimerFd(timerfd_);

    // 获取所有的到期定时器
    std::vector<Entry> expired = getExpired(now);

    // 遍历到期的定时器，执行回调函数
    callingExpiredTimers_ = true; // 当前正在处理回调
    for (const Entry &it : expired) {
        it.second->run();
    }
    callingExpiredTimers_ = false; // 回调处理完毕

    reset(expired, now); // 周期任务处理
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpired;
    // 遍历到期的定时器
    for (const Entry &it : expired) {

        // 如果是重复任务，则重新设置到期时间，并插回定时器队列中
        if (it.second->repeat()) {
            auto timer = it.second;
            timer->restart(Timestamp::now());
            insert(timer);
        } else {
            // 不是重复任务则删除
            delete it.second;
        }

        if (!timers_.empty()) {
            resetTimerfd(timerfd_, (timers_.begin()->second)->expiration());
        }
    }
}

// 真正将定时器插入到set中
bool TimerQueue::insert(Timer *timer) {
    bool earliestChanged = false;
    Timestamp when = timer->expiration(); // 第一次执行的时间
    TimerList::iterator it = timers_.begin();
    // 如果是定时器列表为空或比最早的的时间还早，返回true
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    timers_.insert(Entry(when, timer));

    return earliestChanged;
}