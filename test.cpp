#include <gperftools/profiler.h>
#include <chrono>
#include <thread>

void waste_time() {
    for (int i = 0; i < 100000000; ++i) {
        volatile double x = i * 0.5;
    }
}

int main() {
    ProfilerStart("cpu_profile.prof");

    waste_time();  // 执行一些能占 CPU 的函数

    ProfilerStop();
    return 0;
}
