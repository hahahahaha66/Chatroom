#include "Server.h"
#include "../muduo/logging/Logging.h"
 
void Server::ConnectionCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << "New connection from" << conn->peerAddress().toIpPort();
    }
    else  
    {
        LOG_INFO << "Disconnection from" << conn->peerAddress().toIpPort();
    }
}

void Server::MessageCompleteCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << conn->getLoop() << " Loop and send messages to " << conn->peerAddress().toIpPort();
    }
}

void Server::MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    auto msgopt = codec_.tryDecode(buf);
    if (!msgopt.has_value())
    {
        conn->send("Data sending failed, please resend");
        return ;
    }
    auto [type, seq, js] = msgopt.value();

    dispatcher_.dispatch(type, conn, js, seq, time);
}