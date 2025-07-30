#ifndef LOGGING_H
#define LOGGING_H

#include "../base/Timestamp.h"  //时间戳
#include "LogStream.h"  //日志流

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <functional>

class SourceFile  //截取文件名
{
public:
    explicit SourceFile(const char* filename) : data_(filename)
    {
        const char* slash = strrchr(filename, '/');  //从字符串尾找到第一个/
        if(slash)
        {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
};

//使用LogStream，向其写入日志内容
class Logger
{
public:
    enum LogLevel
    {
        //等级由低到高
        TRACE,  //最细粒度的信息
        DEBUG,  //调试信息
        INFO,   //普通运行信息
        WARN,   //警告信息       
        ERROR,  //错误信息
        FATAL,  //致命错误
        LEVEL_COUNT,  //等级数量
    };

    //多个构造版本，包含日志级别和函数名
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    //关键流，日志通过他来输出，由于重载了<<，可以使用流式插入
    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);  //设置全局日志等级

    using OutputFunc = std::function<void(const char* msg, int len)>;
    using FlushFunc = std::function<void()>;
    //可以通过下面的函数来改变日志的输出
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

private:
    class Impl  //真正的实现者
    {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int savedErrno, const char* file, int line);
        void formatTime();
        void finish();  //一条日志的结尾

        Timestamp time_;  //时间戳
        LogStream stream_;  //存储日志的拼接流
        LogLevel level_;  //日志级别
        int line_;  //行号
        SourceFile basename_;  //文件名
    };

    Impl impl_;
};


extern Logger::LogLevel g_logLevel;  //具有外部链接性，多文件共享变量，全局控制日志等级的变量

inline Logger::LogLevel logLevel()  //获得日志等级
{
    return g_logLevel;
}

const char* getErrnoMsg(int savedErrno);  //错误信息辅助函数

//这里也就是为什么在写日志时直接使用LOG_,使用宏简化了Logger的构造细节
//编译器宏，获取文件名，代码行号，函数名
// \ 表示拼接宏，允许多行书写
#define LOG_DEBUG if (logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO if(logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__).stream()
    
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif