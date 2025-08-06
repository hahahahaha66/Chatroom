#include "../client/Client.h"

#include <csignal>
#include <gperftools/profiler.h>

namespace {
std::function<void()> g_handler; // 不暴露 loop，仅暴露行为
}

void Quit(int sig) {
    std::cout << "检测到信号Ctrl + C" << std::endl;
    ProfilerStop();

    if (g_handler) {
        g_handler(); // 间接调用含有 loop 的 lambda
    }
}

int main() {
    ProfilerStart("cpu_profile.prof");
    EventLoop loop;

    Client client(loop, 10101, "10.30.0.120", "ChatClient");

    client.start();

    std::thread th([&client]() { client.InputLoop(); });

    g_handler = [&client, &th]() {
        client.stop();
        th.detach();
    };
    signal(SIGINT, Quit);

    loop.loop();
    std::cout << "EventLoop 已退出" << std::endl;

    if (th.joinable())
        th.join();

    return 0;
}