#include "server/Server.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/tcp/InetAddress.h"

int main ()
{
    EventLoop* loop;
    InetAddress addr(8080);
    Server server_(loop, addr, "Chatroom");

    server_.StartServer();
}