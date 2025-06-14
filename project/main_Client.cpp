#include "client/Client.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/tcp/InetAddress.h"
#include <atomic>

int main ()
{
    EventLoop loop;
    InetAddress addr(8080, "127.0.0.1");
    int seq = 1;

    Client client(&loop, addr, "ChatClient");

    client.Connect();

    std::thread thr([&loop, &client, &seq]() {
        InitialInterface(&loop, &client, seq);
    });

    loop.loop();

}