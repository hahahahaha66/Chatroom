#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread {
// 保存tid缓冲，避免多次系统调用
extern thread_local int t_cachedTid;

void cacheTid();

inline int tid() {
    //__builtin_expect优化编译指令
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}
} // namespace CurrentThread

#endif