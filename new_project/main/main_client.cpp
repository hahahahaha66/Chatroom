#include "../client/Client.h"

#include <gperftools/profiler.h>
#include <csignal>

namespace {
    std::function<void()> g_handler;  // 不暴露 loop，仅暴露行为
}

void Quit(int sig)
{
    std::cout << "检测到信号Ctrl + C" << std::endl;
    // ProfilerStop();

    if (g_handler) {
        g_handler();  // 间接调用含有 loop 的 lambda
    }
}

int main ()
{
    // ProfilerStart("cpu_profile.prof");
    EventLoop loop;

    g_handler = [&loop]() {
        loop.quit();
        loop.wakeup();
    };
    signal(SIGINT, Quit);

    Client client(loop, 10101, "10.30.0.120", "ChatClient");

    client.start();

    std::thread th([&client]() {
        client.InputLoop();
    });

    loop.loop();
    
    if (th.joinable())
        th.join();

    return 0;
}