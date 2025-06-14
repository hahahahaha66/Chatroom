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
#include <unordered_map>
#include <vector>

class Clientservice  
{
public:
    //消息
    void SendToFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const std::string& message, const int seq);
    void SendToGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const std::string& message, const int seq);

    //登陆注册
    void LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const int seq);
    void RegistrationRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const int seq);

    //聊天历史记录
    void GetPersonalChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& otherid, const int seq);
    void GetGroupChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const int seq);

    //申请
    void SendFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const int seq);
    void SendGroupRequest(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const int seq);

    //处理申请
    void ProcessingFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, bool result, const int seq);
    void ProcessingGroupRequest(const TcpConnectionPtr& conn, const int& groupid, const int& userid, bool result, const int seq);

    //好友
    void DeleteFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const int seq);
    void BlockFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const int seq);

    //群聊
    void QuitGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const int seq);
    void CreateGroup(const TcpConnectionPtr& conn, const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const int seq);
    void RemoveGroupUser(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const int seq);
    void SetAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const int seq);
    void RemoveAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const int seq);
    void DeleteGroup(const TcpConnectionPtr& conn, const int& groupid, const int seq);

    void UpdatedUserInterface(const TcpConnectionPtr& conn, int& peeid, std::string& type, const int seq);

    //处理返包

    bool Back_SendToFriend(const json& js, Timestamp time);
    bool Back_SendToGroup(const json& js, Timestamp time);

    bool Back_LoginRequest(const json& js, Timestamp time);
    bool Back_RegistrationRequest(const json& js, Timestamp time);

    bool Back_GetPersonalChatHistory(const json& js, Timestamp time);
    bool Back_GetGroupChatHistory(const json& js, Timestamp time);

    bool Back_SendFriendRequest(const json& js, Timestamp time);
    bool Back_SendGroupRequest(const json& js, Timestamp time);

    bool ProcessingFriendRequest(const json& js, Timestamp time);
    bool ProcessingGroupRequest(const json& js, Timestamp time);


    bool Back_BlockFriend(const json& js, Timestamp time);
    bool Back_DeleteFriend(const json& js, Timestamp time);
    
    bool Back_QuitGroup(const json& js, Timestamp time);
    bool Back_CreateGroup(const json& js, Timestamp time);
    bool Back_RemoveGroupUser(const json& js, Timestamp time);
    bool Back_SetAdministrator(const json& js, Timestamp time);
    bool Back_RemoveAdministrator(const json& js, Timestamp time);
    bool Back_DeleteGroup(const json& js, Timestamp time);

    bool CommandReply(const json& js, Timestamp time);

    //处理刷新包
    void RefreshMessage(const json& js, Timestamp time);

    void RefreshFriendDelete(const json& js, Timestamp time);
    void RefreshFriendBlock(const json& js, Timestamp time);
    void RefreshFriendAddApply(const json& js, Timestamp time);
    void RefreshGroupAddApply(const json& js, Timestamp time);
    void RefreshGroupCreate(const json& js, Timestamp time);
    void RefreshFriendApplyProcess(const json& js, Timestamp time);
    void RefreshGroupApplyProcess(const json& js, Timestamp time);
    void RefreshGroupQuitUser(const json& js, Timestamp time);
    void RefreshGroupRemoveUser(const json& js, Timestamp time);
    void RefreshGroupAddAdministrator(const json& js, Timestamp time);
    void RefreshGroupRemoveAdministrator(const json& js, Timestamp time);

private:
    Codec codec_;

    ChatConnect chatconnect_;
    int userid_;
    std::string username_;
    std::string password_;
    std::unordered_map<int, SimpUser> friendapplylist;
    std::unordered_map<int, Friend> friendlist_;
    std::unordered_map<int, Group> grouplist_;
};

#endif