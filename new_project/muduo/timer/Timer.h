#ifndef TIMER_H
#define TIMER_H

#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include <functional>

// 用于描述一个定时器，可以是一次性定时器，也可以是周期性执行
// 通俗解释——到某个时间点，或者每隔多久，执行回调函数

class Timer : noncopyable {
  public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)), expiration_(when), interval_(interval),
          repeat_(interval > 0.0) {}

    void run() const { callback_(); }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(Timestamp now); // 设置周期定时器下一次超时时间

  private:
    const TimerCallback callback_; // 回调函数
    Timestamp expiration_;         // 第一次执行的时间
    const double interval_; // 反复执行的时间间隔，如果是一次性定时器，则为0
    const bool repeat_;     // 是否是周期性定时器——每隔一定时间反复执行任务
};

#endif