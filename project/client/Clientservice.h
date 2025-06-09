#ifndef CLIENTSERVICE_H
#define CLIENTSERVICE_H

#include "../json_protocol.hpp"
#include "../muduo/net/tcp/TcpClient.h"
#include "../tool/Codec.h"
#include "../entity/Friend.h"
#include "../entity/GroupUser.h"
#include "../entity/Group.h"

#include <cstdint>
#include <string>
#include <vector>

class Clientservice  
{
public:
    //消息
    void SendToFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const std::string& message, const uint16_t seq);
    void SendToGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const std::string& message, const uint16_t seq);

    //登陆注册
    void LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq);
    void RegistrationRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq);

    //聊天历史记录
    void GetPersonalChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& otherid, const uint16_t seq);
    void GetGroupChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);

    //申请
    void SendFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);
    void SendGroupRequest(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);

    //处理申请
    void ProcessingFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, bool result, const uint16_t seq);
    void ProcessingGroupRequest(const TcpConnectionPtr& conn, const int& groupid, const int& userid, bool result, const uint16_t seq);

    //好友
    void DeleteFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);
    void BlockFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);

    //群聊
    void QuitGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);
    void CreateGroup(const TcpConnectionPtr& conn, const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const uint16_t seq);
    void RemoveGroupUser(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void SetAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void RemoveAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void DeleteGroup(const TcpConnectionPtr& conn, const int& groupid, const uint16_t seq);

    void UpdatedUserInterface(const TcpConnectionPtr& conn, int& peeid, std::string& type, const uint16_t seq);

    //处理返包

    void Back_SendToFriend(const json& js, Timestamp time);
    void Back_SendToGroup(const json& js, Timestamp time);

    void Back_LoginRequest(const json& js, Timestamp time);
    void Back_RegistrationRequest(const json& js, Timestamp time);

    void Back_GetPersonalChatHistory(const json& js, Timestamp time);
    void Back_GetGroupChatHistory(const json& js, Timestamp time);

    void Back_SendFriendRequest(const json& js, Timestamp time);
    void Back_SendGroupRequest(const json& js, Timestamp time);

    void ProcessingFriendRequest(const json& js, Timestamp time);
    void ProcessingGroupRequest(const json& js, Timestamp time);


    void Back_BlockFriend(const json& js, Timestamp time);
    void Back_DeleteFriend(const json& js, Timestamp time);
    
    void Back_QuitGroup(const json& js, Timestamp time);
    void Back_CreateGroup(const json& js, Timestamp time);
    void Back_RemoveGroupUser(const json& js, Timestamp time);
    void Back_SetAdministrator(const json& js, Timestamp time);
    void Back_RemoveAdministrator(const json& js, Timestamp time);
    void Back_DeleteGroup(const json& js, Timestamp time);

    void CommandReply(const json& js, Timestamp time);



private:
    Codec codec_;

    ChatConnect chatconnect_;
    int userid;
    std::string username;
    std::string password;
    std::unordered_map<int, Friend> friendlist_;
    std::unordered_map<int, Group> grouplist_;
    std::atomic<long> total_seq;
};

#endif