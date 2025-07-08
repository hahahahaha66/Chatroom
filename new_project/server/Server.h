#pragma once
#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../tool/Dispatcher.h"
#include "../tool/Codec.h"
#include "service/UserService.h"

class Server 
{
public:
    Server(EventLoop* loop, const InetAddress& listenAddr);

    void start();

private:
    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    void ThreadInitCallback(EventLoop* loop);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);

    TcpServer server_;
    Codec codec_;
    Dispatcher dispatcher_;

    UserService userservice_;
};