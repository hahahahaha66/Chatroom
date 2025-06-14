#ifndef SERVER_H
#define SERVER_H

#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../muduo/net/tcp/TcpConnection.h"
#include "Service.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include "../muduo/logging/Logging.h"
#include "../database/MysqlConnectionpool.h"

#include <functional>
#include <mysql/mysql.h>

class Server 
{
public:
    Server(EventLoop* loop, InetAddress& addr, std::string name)
        : loop_(loop),
          server_(loop, addr, name),
          service_(dispatcher_)
    {
        LOG_INFO << "Server initializing";

        MysqlConnectionPool::Instance().Init("localhost", 3306, "chatroom_user", "StrongPassword123!", "Chatroombase", 10);
        
        server_.setConnectionCallback(std::bind(&Server::ConnectionCallback, this, _1));
        server_.setMessageCallback(std::bind(&Server::MessageCallback, this, _1, _2, _3));
        server_.setWriteCompleteCallback(std::bind(&Server::MessageCompleteCallback, this, _1));
        server_.setThreadInitCallback(std::bind(&Server::ThreadInitCallback, this, _1));

        service_.RegisterAllHanders(dispatcher_);

        LOG_INFO << "Server initialization completed";

        LOG_INFO << "Listening started " << server_.inPort();

        StartServer();
    }

    ~Server() {}

    void StartServer()
    {
        server_.start();
    }

    void setThreadnum(int& num)
    {
        server_.setThreadNum(num);
    }

    void ConnectionCallback(const TcpConnectionPtr& conn);
    void MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);
    void ThreadInitCallback(EventLoop* loop);

private:
    EventLoop* loop_;
    TcpServer server_;
    Codec codec_;
    Dispatcher dispatcher_;
    Service service_;
};

#endif