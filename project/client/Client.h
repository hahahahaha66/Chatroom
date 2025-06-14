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
#include <unordered_map>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Client
{
public:
    Client(EventLoop* loop, InetAddress& addr, std::string name);
    ~Client();
    
    void ConnectionCallback(const TcpConnectionPtr& conn);
    void MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void WriteCompleteCallback(const TcpConnectionPtr& conn);

    void Connect()
    {
        client_.connect();
    }

    std::weak_ptr<Clientservice> Getclientservice()
    {
        return clientservice_;
    }

    TcpClient* GetTcpclient()
    {
        return &client_;
    }

    Processpend* GetProcessend()
    {
        return &processpend_;
    }

private:
    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr conn_;

    Codec codec_;
    Processpend processpend_;
    std::shared_ptr<Clientservice> clientservice_;
};

void InitialInterface(EventLoop* loop, Client* client, int &seq);
void MainInterface(EventLoop* loop, Client* client, int &seq);
void FriendInterface(EventLoop* loop, Client* client, int &seq);
void GroupInterface(EventLoop* loop, Client* client, int &seq);

void PrivateChatInterface(EventLoop* loop, Client* client, int &seq, int friendid);
void GroupChatInterface(EventLoop* loop, Client* client, int &seq, int groupid);

void ClearScreen();

bool ReadingIntegers(std::string input, int &result);

#endif