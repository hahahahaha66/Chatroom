#include "AsyncLogging.h"

// 整个缓冲区分为前端线程和后端线程
// 前端线程有一个正在写入的currentBuffer缓冲区，还有一个nextBuffer备用缓冲区
// 一个写满将内容提交到buffer,并换成另一个，如果都被写满（少），则重新分配currentBuffer
// 以上是前端快写
// 接下来是后端慢写，启用另一线程
// 在程序开始创建newBuffer1和newBuffer2,用于替换currentBuffer和nextBuffer
// 用bufferToWrite读取buffer，在这里实现真正的写入磁盘
// 将bufferToWrite设为2,防止分配过大内存，同时提供newBuffer1和newBuffer2的复用

AsyncLogging::AsyncLogging(const std::string &basename, off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval), running_(false), basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"), mutex_(),
      cond_(), currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_() {
    // bzero相当于重置缓冲区，将数据清零，把指针重置到开头
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16); // 预留16个元素的大小，防止频繁realloc
}

void AsyncLogging::append(const char *logline, int len) {
    std::lock_guard<std::mutex> lock(mutex_);

    // if (len <= 0 || (strspn(logline, "\r\n\t ") == len)) {
    //     return;  // 忽略空日志内容
    // }

    if (currentBuffer_->avail() > len) // 如果缓冲区剩余空间足够，直接写入
    {
        currentBuffer_->append(logline, len);
    } else // 如果不够，写入nextBuffer
    {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) // 检测nextBuffer_是否写满，没写满直接转入currentBuffer
        {
            currentBuffer_ = std::move(nextBuffer_);
        } else // 写满了就重新分配currentBuffer
        {
            currentBuffer_.reset(new Buffer); // 实质上是把指针重置，重用内存
        }

        currentBuffer_->append(logline, len);

        cond_.notify_one(); // 唤醒线程
    }
}

void AsyncLogging::threadFunc() {
    LogFile output(basename_, rollSize_, flushInterval_); // 创建日志文件

    // 对象池思想，创建两个缓冲区，避免频繁的创建和销毁buffer
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(
        16); // 用来存储buffer,原buffer是共享资源，前后端都有可能访问
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(3));
            }

            buffers_.push_back(
                std::move(currentBuffer_)); // 将当前缓冲区内容存进buffers

            currentBuffer_ = std::move(newBuffer1);

            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        for (const auto &buffer : buffersToWrite) // 真正的写入磁盘
        {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) // 限制缓存数量为多有两个
        {
            buffersToWrite.resize(2);
        }

        // 从buffersToWrite中回收两个buffer作备用缓冲
        if (!newBuffer1) {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2) {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}
