#include "../server/Server.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"

int main ()
{
    EventLoop loop;
    InetAddress addr(10101, "0.0.0.0");

    Server server_(&loop, addr, "Chatroom");

    server_.start();
    loop.loop();

    return 0;
}