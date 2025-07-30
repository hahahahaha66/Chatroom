#include "Poller.h"
#include "EPollPoller.h"

#include <cstdlib>
#include <stdlib.h>

//设置默认的Poller的实现方式
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else 
    {
        return new EPollPoller(loop);
    }
}