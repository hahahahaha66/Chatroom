#include <gperftools/profiler.h>

int main() {
    ProfilerStart("cpu_profile.prof");
    for (volatile int i = 0; i < 100000000; ++i) {
        int x = i * i;
    }
    ProfilerStop();
    return 0;
}
