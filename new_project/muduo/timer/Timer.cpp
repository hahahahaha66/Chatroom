#include "Timer.h"

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = addTime(now, interval_);
    }
    else 
    {
        //一次性定时器，设置为无效时间
        expiration_ = Timestamp();
    }
}