#include "client/Client.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/tcp/InetAddress.h"

int main ()
{
    EventLoop* loop;
    InetAddress addr(8080);

    Client client(loop, addr, "ChatClient");

    
}