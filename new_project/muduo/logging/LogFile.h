#ifndef LOGFILE_H
#define LOGFILE_H

#include "FileUtil.h"

#include <ctime>
#include <mutex>
#include <memory>

class LogFile
{
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            int flushInterval = 3,
            int checkEveryN = 1024
            );

    ~LogFile();

    void append(const char* data, int len);  //写入日志数据
    void flush();  //刷盘，把缓冲区数据写入磁盘
    bool rollFile();  //滚动日志文件

private:
    static std::string getLogFileName(const std::string& basename, time_t* now);
    void appendInLock(const char* data, int len);

    const std::string basename_;  //日志文件名
    const off_t rollSize_;  //日志文件最大字节数
    const int flushInterval_;  //定期刷新文件
    const int checkEveryN_;  //每写入N条日志检查一次是否需要滚动或刷新

    int count_;  //记录当前已写日志条数

    std::unique_ptr<std::mutex> mutex_;
    time_t startOfPeriod_;  //上一次日志文件生成的零点时间
    time_t lastRoll_;  //上一次日志文件生成的时间
    time_t lastFlush_;  //上一次刷新的时间
    std::unique_ptr<FileUtil> file_;  //使用封装好的写入刷新操作

    const static int kRollPerSeconds_ = 60*60*24;
};
#endif