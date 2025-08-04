#include "Logging.h"

#include "LogStream.h"
#include <cstdio>
#include <ctime>

namespace ThreadInfo
{
    //每一个线程自己的存储错误，时间的空间
    __thread char t_errnobuf[512];
    __thread char t_time[64];
    __thread time_t t_lastSecond;
};

const char* getErrnoMsg(int saveErrno)
{
    return strerror_r(saveErrno, ThreadInfo::t_errnobuf, sizeof(ThreadInfo::t_errnobuf));
}

const char* const getLevelName[Logger::LogLevel::LEVEL_COUNT] = 
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};

Logger::LogLevel initLogLevel()
{
    return Logger::INFO;  //默认日志等级, 通过此来修改日志输出
}

Logger::LogLevel g_logLevel = initLogLevel();

static void defaultOutput(const char* data, int len)
{
    fwrite(data, len, sizeof(char), stdout);  //默认日志输出
}

static void defaultFlush()
{
    fflush(stdout);  //默认日志刷新
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(Logger::LogLevel level, int savedErrno, const char* file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file)
{
    formatTime();  //写入当前时间

    stream_ << GeneralTemplate(getLevelName[level], 6) << " ";  //写入日志等级
    if (savedErrno != 0)  //检查有无错误，有则输出
    {
        stream_ << getErrnoMsg(savedErrno) << " (errno=" << savedErrno << ")";
    }
}

void Logger::Impl::formatTime()
{
    Timestamp now = Timestamp::now();  //获取当前时间
    time_t seconds = static_cast<time_t>(now.microSecondsSinceEpoch() / Timestamp::kMicroSecondsPerSecond);  //获取秒数
    int microsecond = static_cast<int>(now.microSecondsSinceEpoch() % Timestamp::kMicroSecondsPerSecond);  //获取微秒数

    struct tm *tm_time = localtime(&seconds);

    snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);

    ThreadInfo::t_lastSecond = seconds;

    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%06d ", microsecond);

    stream_ << GeneralTemplate(ThreadInfo::t_time, 19) << '.' << GeneralTemplate(buf, 7);
}

void Logger::Impl::finish()
{
    stream_ << " - " << GeneralTemplate(basename_.data_, basename_.size_)
            << ':'  << line_ << '\n';
}

Logger::Logger(const char* file, int line) 
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, Logger::LogLevel level)
    : impl_(level, 0, file, line)
{
}

Logger::Logger(const char* file, int line, Logger::LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::~Logger()
{
    impl_.finish();  //补全日志信息

    const LogStream::Buffer& buf(stream().buffer());

    g_output(buf.data(), buf.length());

    if(impl_.level_ == FATAL)  //若检测到致命错误，崩溃程序
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}
