#include "../server/Server.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"

#include <csignal>
#include <gperftools/profiler.h>

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
    InetAddress chataddr(10101, "0.0.0.0");
    InetAddress fileaddr(10102, "0.0.0.0");

    g_handler = [&loop]() {
        loop.quit();
    };
    signal(SIGINT, Quit);

    Server server_(&loop, chataddr, fileaddr, "Chatroom");

    server_.start();
    loop.loop();

    return 0;
}