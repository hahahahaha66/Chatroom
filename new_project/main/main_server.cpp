#include "../server/Server.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"

int main ()
{
    EventLoop loop;
    InetAddress chataddr(10101, "0.0.0.0");
    InetAddress fileaddr(10102, "0.0.0.0");

    Server server_(&loop, chataddr, fileaddr, "Chatroom");

    server_.start();
    loop.loop();

    return 0;
}