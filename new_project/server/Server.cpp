#include "Server.h"
#include "../muduo/logging/Logging.h"

Server::Server(EventLoop* loop,const InetAddress& listenAddr)
    : server_(loop, listenAddr, "ChatServer"), 
      dispatcher_() 
{
    server_.setConnectionCallback(std::bind(&Server::OnConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&Server::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void Server::start()
{
    server_.start();
}

void Server::ThreadInitCallback(EventLoop* loop)
{
    
}

void Server::MessageCompleteCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << conn->getLoop() << " Loop and send messages to " << conn->peerAddress().toIpPort();
    }
}

void Server::OnConnection(const TcpConnectionPtr& conn) 
{
    if (conn->connected()) 
    {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    } 
    else
    {
        LOG_INFO << "Connection disconnected: " << conn->peerAddress().toIpPort();
    }
}

void Server::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
{
    auto msgopt = codec_.tryDecode(buffer);

    if (!msgopt.has_value())
    {
        LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
    }

    auto [type, js] = msgopt.value();

    dispatcher_.dispatch(type, conn, js, time);
}