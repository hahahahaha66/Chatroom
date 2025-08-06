#ifndef THREAD_H
#define THREAD_H

#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <sched.h>
#include <semaphore.h>
#include <string>
#include <thread>
#include <unistd.h>

class Thread : noncopyable {
  public:
    using ThreadFunc = std::function<void()>;
    // ecplicit修饰构造函数，防止隐形构造
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start(); // 开启线程
    void join();  // 等待线程

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string &name() const { return name_; }

    static int numCreated() { return numCreated_; }

  private:
    void setDefaultName();

    bool started_;                          // 是否启动线程
    bool joined_;                           // 是否等待线程
    std::shared_ptr<std::thread> thread_;   // 指向线程
    pid_t tid_;                             // 线程id
    ThreadFunc func_;                       // 调用的回调函数
    std::string name_;                      // 线程名
    static std::atomic_int32_t numCreated_; // 线程索引
};

#endif