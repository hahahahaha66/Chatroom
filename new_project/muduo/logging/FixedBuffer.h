#ifndef FIXEDBUFFER_H
#define FIXEDBUFFER_H

#include "../base/noncopyable.h"
#include <assert.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <strings.h>

constexpr const int kSmallBuffer = 4000;        // 前端拼接的小缓冲区
constexpr const int kLargeBuffer = 4000 * 1000; // 后端异步的大缓冲区

// 提供基础缓冲区，用于存储数据
template <int SIZE> class FixedBuffer : noncopyable {
  public:
    FixedBuffer() : cur_(data_) {}

    // 追加缓冲区字符串
    void append(const char *buf, size_t len) {
        assert(len <= static_cast<size_t>(avail()));

        if (static_cast<size_t>(avail()) >= len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char *data() const { return data_; } // 获取缓冲区指针
    int length() const {
        return static_cast<int>(cur_ - data_);
    } // 获取缓冲区长度

    char *current() { return cur_; } // 获得缓冲区当前字符
    int avail() const {
        return static_cast<int>(end() - cur_);
    } // 获取当前缓冲区的剩余长度
    void add(size_t len) { cur_ += len; } // 缓冲区向后推移len长度字符

    void reset() { cur_ = data_; }                    // 将缓冲区重置
    void bzero() { memset(data_, 0, sizeof(data_)); } // 将指定内存清零

    std::string toString() const {
        return std::string(data_, length());
    } // 返回当前缓冲内容

  private:
    const char *end() const {
        return data_ + sizeof(data_);
    } // 获取当前缓冲区的尾位置

    char data_[SIZE]; // 缓冲区
    char *cur_;       // 指向当前字符串的位置
};

#endif