#include "server/Server.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/tcp/InetAddress.h"

int main ()
{
    EventLoop loop;
    InetAddress addr(8080, "127.0.0.1");

    Server server_(&loop, addr, "Chatroom");

    server_.StartServer();
    loop.loop();

    return 0;
}