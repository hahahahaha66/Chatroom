#include "CurrentThread.h"

namespace CurrentThread {
thread_local int t_cachedTid = 0;

void cacheTid() {
    if (t_cachedTid == 0) {
        // 系统调用，获取线程id
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
} // namespace CurrentThread