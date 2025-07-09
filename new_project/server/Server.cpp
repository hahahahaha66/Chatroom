#include "Server.h"
#include "../muduo/logging/Logging.h"
#include <functional>

Server::Server(EventLoop* loop,const InetAddress& listenAddr, std::string name)
    : server_(loop, listenAddr, name), 
      dispatcher_() 
{
    MysqlConnectionPool::Instance().Init("localhost", 3306, "hahaha", "123456", "chatroom", 10);

    server_.setConnectionCallback(std::bind(&Server::OnConnection, this, _1));
    server_.setMessageCallback(std::bind(&Server::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(std::bind(&Server::MessageCompleteCallback, this, _1));
    server_.setThreadInitCallback(std::bind(&Server::ThreadInitCallback, this, _1));

    dispatcher_.registerHander("Register", std::bind(&Service::UserRegister, &service_, _1, _2, _3));

    dispatcher_.registerHander("Login", std::bind(&Service::UserLogin, &service_, _1, _2, _3));

    dispatcher_.registerHander("SendMessage", std::bind(&Service::MessageSend, &service_, _1, _2, _3));

    dispatcher_.registerHander("ChatHistory", std::bind(&Service::GetChatHistory, &service_, _1, _2, _3));
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
    LOG_INFO << "tryDecode: buffer size = " << buffer->readableBytes();
    
    auto msgopt = codec_.tryDecode(buffer);

    if (!msgopt.has_value())
    {
        LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
        return ;
    }

    auto [type, js] = msgopt.value();

    dispatcher_.dispatch(type, conn, js, time);
}