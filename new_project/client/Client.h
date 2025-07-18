#pragma once

#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

struct Friend {
    Friend(int id, std::string name, bool block)
        : id_(id), name_(name), block_(block) {}
    Friend() : id_(0), name_(""), block_(false) {}
    int id_;
    std::string name_;
    bool block_;
};

struct FriendApply {
    FriendApply(int id, std::string name, std::string status)
        : id_(id), name_(name), status_(status) {}
    int id_;
    std::string name_;
    std::string status_;
};

struct Group {
    Group(int id, std::string name, std::string role)
        : id_(id), name_(name), role_(role) {}
    int id_;
    std::string name_;
    std::string role_;
};

class Client 
{
public:
    Client(EventLoop& loop, InetAddress addr, std::string name);
    
    void ConnectionCallBack(const TcpConnectionPtr& conn);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);

    void Register(const json& js);
    void Login(const json& js);
    void RegisterBack(const TcpConnectionPtr& conn, const json& js);
    void LoginBack(const TcpConnectionPtr& conn, const json& js);

    // 消息
    void SendMessage(const json& js);
    void SendApply(const json& js);
    void SendMessageBack(const TcpConnectionPtr& conn, const json& js);
    void RecvMessageBack(const TcpConnectionPtr& conn, const json& js);
    void GetChatHistory(const json& js);
    void GetChatHistoryBack(const TcpConnectionPtr& conn, const json& js);
    void GetGroupHistory(const json& js);
    void GetGroupHistoryBack(const TcpConnectionPtr& conn, const json& js);

    // 好友
    void ListAllApply(const json& js);
    void ListAllSendApply(const json& js);
    void ProceFriendApply(const json& js);
    void ListFriend(const json& js);

    void SendFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListSendFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ProceFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListFriendBack(const TcpConnectionPtr& conn, const json& js);

    // 群聊
    void CreateGroup(const json& js);
    void SendGroupApply(const json& js);
    void ListGroupApply(const json& js);
    void ProceGroupApply(const json& js);
    void ListSendGroupApply(const json& js);
    void GroupMember(const json& js);
    void ListGroup(const json& js);
    void QuitGroup(const json& js);
    void ChangeUserRole(const json& js);

    void CreateGroupBack(const TcpConnectionPtr& conn, const json& js);
    void SendGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ProceGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListSendGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void GroupMemberBack(const TcpConnectionPtr& conn, const json& js);
    void ListGroupBack(const TcpConnectionPtr& conn, const json& js);
    void QuitGroupBack(const TcpConnectionPtr& conn, const json& js);
    void ChangeUserRoleBack(const TcpConnectionPtr& conn, const json& js);

    void start();
    void stop();
    void send(const std::string& msg);
    inline void waitInPutReady();
    inline void notifyInputReady();

    void InputLoop();

private:
    std::atomic<bool> waitingback_ = false;
    std::string currentState_ = "init_menu";
    std::mutex mutex_;
    std::condition_variable cv_;

    TcpClient client_;
    Codec codec_;
    Dispatcher dispatcher_;

    int userid_ = 0;
    std::string email_;
    std::string name_;
    std::string password_;

    std::unordered_map<int, Friend> friendlist_;
    std::unordered_map<int, FriendApply> friendapplylist_;
    std::unordered_map<int, FriendApply> friendsendapplylist_;
    std::unordered_map<int, Group> grouplist_;
};
