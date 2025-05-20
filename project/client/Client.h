#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"

#include <string>

class Client
{
public:
    Client(EventLoop* loop, InetAddress& addr, std::string name)
        : loop_(loop),
          client_(loop, addr, name)
    {

    }



private:
    EventLoop* loop_;
    TcpClient client_;
    Codec codec_;
    
};