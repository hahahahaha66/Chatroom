#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../muduo/net/tcp/TcpConnection.h"

#include <functional>

class Server 
{
public:
    Server(EventLoop* loop, InetAddress& addr, std::string name)
        : loop_(loop),
          server_(loop, addr, name)
    {
        server_.setConnectionCallback(std::bind(&Server::ConnectionCallback, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&Server::MessageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setWriteCompleteCallback(std::bind(&Server::MessageCompleteCallback, this, std::placeholders::_1));
        server_.setThreadInitCallback(std::bind(&Server::ThreadInitCallback, this, std::placeholders::_1));
    }

    ~Server() {}

    void setThreadnum(int& num)
    {
        server_.setThreadNum(num);
    }

private:
    void ConnectionCallback(const TcpConnectionPtr& conn);
    void MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);
    void ThreadInitCallback(EventLoop* loop);

    EventLoop* loop_;
    TcpServer server_;

};