#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <cstdint>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>
#include <sys/time.h>
#include <inttypes.h> //包含PRId64

class Timestamp
{
public:
    Timestamp() : micioSecondsSinceEpoch_(0)
    {
    }

    explicit Timestamp(int64_t micioSecondsSinceEpoch)
        : micioSecondsSinceEpoch_(micioSecondsSinceEpoch)
    {
    }

    //获取当前时间戳
    //静态函数，无须类内成员就可以获取当前时间
    static Timestamp now();

    //以string形式返回，格式[millisec].[microsec]
    std::string toString() const;
    //格式 "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
    std::string toFormattedString(bool showMicroseconds = false) const;

    //返回当前时间戳的微秒
    int64_t microSecondsSinceEpoch() const { return micioSecondsSinceEpoch_; }
    //返回当前时间戳的秒数
    int64_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(micioSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    //失效的时间戳
    static Timestamp invalid()
    {
        return Timestamp();
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    //表示自开始时间戳的秒数
    int64_t micioSecondsSinceEpoch_;
};

//用于比较时间，重载运算符
inline bool operator<(Timestamp lhs, Timestamp rht)
{
    return lhs.microSecondsSinceEpoch() < rht.microSecondsSinceEpoch();
} 


inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

//重复定时任务会对次时间戳进行增加
inline Timestamp addTime(Timestamp timestamp, double second)
{   
    //增加的时间转换成微秒
    int64_t delta = static_cast<int64_t>(second * Timestamp::kMicroSecondsPerSecond);
    //返回新增后的时间戳
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

#endif