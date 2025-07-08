#include "Server.h"
#include "../muduo/logging/Logging.h"
#include <functional>

Server::Server(EventLoop* loop,const InetAddress& listenAddr)
    : server_(loop, listenAddr, "ChatServer"), 
      dispatcher_() 
{
    server_.setConnectionCallback(std::bind(&Server::OnConnection, this, _1));
    server_.setMessageCallback(std::bind(&Server::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(std::bind(&Server::MessageCompleteCallback, this, _1));

    dispatcher_.registerHander("Register", [this](const std::shared_ptr<TcpConnection>& conn,
        const nlohmann::json& json, Timestamp time) {
        this->service_.UserRegister(conn, json, time);
    });

    dispatcher_.registerHander("Login", [this](const std::shared_ptr<TcpConnection>& conn,
        const nlohmann::json& json, Timestamp time) {
        this->service_.UserLogin(conn, json, time);
    });

    dispatcher_.registerHander("SendMessage", [this](const std::shared_ptr<TcpConnection>& conn,
        const nlohmann::json& json, Timestamp time) {
        this->service_.MessageSend(conn, json, time);
    });

    dispatcher_.registerHander("ChatHistory", [this](const std::shared_ptr<TcpConnection>& conn,
        const nlohmann::json& json, Timestamp time) {
        this->service_.GetChatHistory(conn, json, time);
    });
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