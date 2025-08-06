#include "Timestamp.h"
#include <cstdint>
#include <cstdio>

// 获取当前时间戳
Timestamp Timestamp::now() {
    struct timeval tv;

    // 获取微秒和秒
    gettimeofday(&tv, NULL);
    int64_t second = tv.tv_sec;

    // 转换成微秒
    return Timestamp(second * Timestamp::kMicroSecondsPerSecond + tv.tv_usec);
}

// 格式化字符串 [秒].[微秒]
std::string Timestamp::toString() const {
    char buf[32] = {0};
    int64_t seconds = micioSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = micioSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds,
             microseconds);
    return buf;
}

// 格式化字符串 年/月/日 时间
std::string Timestamp::toFormattedString(bool showMicroseconds) const {
    char buf[64] = {0};
    time_t seconds =
        static_cast<time_t>(micioSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    // 使用localtime函数把秒数格式化成日历时间
    tm *tm_time = localtime(&seconds);
    // 是否显示微秒
    if (showMicroseconds) {
        int microsecond =
            static_cast<int>(micioSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                 tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                 tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec,
                 microsecond);
    } else {
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
                 tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    }

    return buf;
}
