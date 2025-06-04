#ifndef CLIENT_H
#define CLIENT_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include "../tool/Processpend.h"
#include "Clientservice.h"

#include <string>
#include <atomic>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Client
{
public:
    Client(EventLoop* loop, InetAddress& addr, std::string name);
    
    void ConnectionCallback(const TcpConnectionPtr& conn);
    void MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void WriteCompleteCallback(const TcpConnectionPtr& conn);

    void InitialInterface();
    void MainInterface();
    void FriendInterface();
    void GroupInterface();
    void PrivateChatInterface(int friendid);
    void GroupChatInterface(int groupid);

    void ClearScreen() 
    {
        std::cout << "\033[2J\033[H";
    }

private:
    EventLoop* loop_;
    TcpClient client_;
    Codec codec_;
    Processpend processpend_;
    Clientservice clientservice_;

    std::atomic<long> total_seq;
};

#endif