#include "LogFile.h"
#include "FileUtil.h"
#include <ctime>
#include <memory>

LogFile::LogFile(const std::string &basename, off_t rollrize, int flushInterval,
                 int checkEveryN)
    : basename_(basename), rollSize_(rollrize), flushInterval_(flushInterval),
      checkEveryN_(checkEveryN), count_(0), mutex_(new std::mutex),
      startOfPeriod_(0), lastRoll_(0), lastFlush_(0) {
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char *data, int len) {
    std::lock_guard<std::mutex> lock(*mutex_);
    appendInLock(data, len);
}

void LogFile::appendInLock(const char *data, int len) {
    file_->append(data, len);

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod != startOfPeriod_) // 跨天自动生成新的日志文件
            {
                rollFile();
            } else if (now - lastFlush_ >
                       flushInterval_) // 太久没刷新，自动刷新
            {
                lastFlush_ = now;
                file_->flush(); // 主动刷新
            }
        }
    }
}

void LogFile::flush() // 手动刷新
{
    file_->flush();
}

bool LogFile::rollFile() // 创建新日志文件
{
    time_t now = ::time(NULL);
    // 把当前的秒数除一天的秒数，由于计算机是整数除法，余数，也就是当天的秒数就被舍去了，再乘回来，就是当天零点的时间
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    std::string filename = getLogFileName(basename_, &now); // 获取新日志文件名

    if (basename_ == "stdout") {
        filename = "stdout";
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil(filename)); // 同时重建FileUtil
        return true;
    } else {
        filename = getLogFileName(basename_, &now); // 获取新日志文件名

        if (now > lastRoll_) // 防止一秒多次创建
        {
            lastRoll_ = now;
            lastFlush_ = now;
            startOfPeriod_ = start;
            file_.reset(new FileUtil(filename)); // 同时重建FileUtil
            return true;
        }
    }

    return false;
}

// 生成新文件名
std::string LogFile::getLogFileName(const std::string &basename, time_t *now) {
    std::string filename;
    filename.reserve(basename.size() + 64);

    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm);

    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    filename += ".txt";

    return filename;
}