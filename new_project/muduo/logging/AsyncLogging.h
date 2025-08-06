#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include "../base/Thread.h"
#include "../base/noncopyable.h"
#include "FixedBuffer.h"
#include "LogFile.h"
#include "LogStream.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <vector>

class AsyncLogging {
  public:
    AsyncLogging(const std::string &basename, // 文件名
                 off_t rollSize,              // 每个文件最大字节数
                 int flushInterval = 3        // 默认刷新时间
    );

    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }

    void append(const char *logling, int len);

    void start() {
        running_ = true;
        thread_.start();
    }

    void stop() {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }

  private:
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    void threadFunc();

    const int flushInterval_;    // 默认刷新时间
    std::atomic<bool> running_;  // 线程退出的标志位，原子变量，线程安全
    const std::string basename_; // 日志文件名
    const off_t rollSize_;       // 日志文件最大字节数
    Thread thread_;              // 线程变量
    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currentBuffer_; // 当前Buffer数组
    BufferPtr nextBuffer_;    // 预备缓冲区，如果当前缓冲区写满了，就用这个
    BufferVector buffers_;    // 已写满的缓冲区集合，用来一次性写入磁盘
};
#endif