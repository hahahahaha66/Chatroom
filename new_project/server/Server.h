#pragma once
#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../tool/Dispatcher.h"
#include "../tool/Codec.h"
#include "Service.h"
#include "../ftp/FileServer.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Server 
{
public:
    Server(EventLoop* loop, const InetAddress& listenAddr, const InetAddress& fileAddr, std::string name);

    void start();

private:
    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    void ThreadInitCallback(EventLoop* loop);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);

    TcpServer server_;
    FileServer fileserver_;
    Codec codec_;
    Dispatcher dispatcher_;

    Service service_;
};