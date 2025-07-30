#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "../base/noncopyable.h"
#include "FixedBuffer.h"

#include <string>
#include <bits/types.h>

class GeneralTemplate : noncopyable
{
public:
    GeneralTemplate() : data_(nullptr), len_(0)
    {
    }

    explicit GeneralTemplate(const char* data, int len) : data_(data), len_(len)
    {
    }

    const char* data_;
    int len_;
};

//建立流式函数，通过重载<<来实现流式传输
class LogStream : noncopyable
{
public:
    using Buffer = FixedBuffer<kSmallBuffer>;

    void append(const char* date, int len) { buffer_.append(date, len); }  //追加日志
    const Buffer& buffer() const { return buffer_; }  //获取完整的日志
    void resetBuffer() { buffer_.reset(); }     //重置日志
    
    void formatPointer(const void* data);

    //以下是重载数字及字符，确保所有的输入都能成功到缓冲区内
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(float v);
    LogStream& operator<<(double v);

    LogStream& operator<<(char c);
    LogStream& operator<<(const void* data);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string& str);
    LogStream& operator<<(const Buffer& buf);

    LogStream& operator<<(const GeneralTemplate& g);

private:
    static const int kMaxNumericSize = 48;  //用于将整书和浮点数格式化转化为字符串时，预留的最大缓冲区

    template<typename T>
    void formatInteger(T);  //将数字写入缓冲区

    Buffer buffer_;  //缓冲区
};

#endif