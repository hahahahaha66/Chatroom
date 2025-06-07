#ifndef CLIENTSERVICE_H
#define CLIENTSERVICE_H

#include "../json_protocol.hpp"
#include "../muduo/net/tcp/TcpClient.h"
#include "../tool/Codec.h"

#include <cstdint>
#include <string>
#include <vector>

class Clientservice  
{
public:
    void SendToFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const std::string& message, const uint16_t seq);
    void SendToGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const std::string& message, const uint16_t seq);

    void LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq);
    void RegistrationRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq);

    void GetPersonalChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& otherid, const uint16_t seq);
    void GetGroupChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);

    void SendFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);
    void SendGroupRequest(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);

    void ProcessingFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, bool result, const uint16_t seq);
    void ProcessingGroupRequest(const TcpConnectionPtr& conn, const int& groupid, const int& userid, bool result, const uint16_t seq);

    void DeleteFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);
    void BlockFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq);

    void QuitGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq);
    void CreateGroup(const TcpConnectionPtr& conn, const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const uint16_t seq);
    void RemoveGroupUser(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void SetAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void RemoveAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq);
    void DeleteGroup(const TcpConnectionPtr& conn, const int& groupid, const uint16_t seq);

    void Back_SendToFriend(const json& js, Timestamp);
    void Back_SendToGroup(const json& js, Timestamp);

    void Back_LoginRequest(const json& js, Timestamp);
    void Back_RegistrationRequest(const json& js, Timestamp);

    void Back_GetPersonalChatHistory(const json& js, Timestamp);
    void Back_GetGroupChatHistory(const json& js, Timestamp);

    void Back_SendFriendRequest(const json& js, Timestamp);
    void Back_SendGroupRequest(const json& js, Timestamp);

    void ProcessingFriendRequest(const json& js, Timestamp);
    void ProcessingGroupRequest(const json& js, Timestamp);


    void Back_BlockFriend(const json& js, Timestamp);
    void Back_DeleteFriend(const json& js, Timestamp);
    
    void Back_QuitGroup(const json& js, Timestamp);
    void Back_CreateGroup(const json& js, Timestamp);
    void Back_RemoveGroupUser(const json& js, Timestamp);
    void Back_SetAdministrator(const json& js, Timestamp);
    void Back_RemoveAdministrator(const json& js, Timestamp);
    void Back_DeleteGroup(const json& js, Timestamp);

private:
    Codec codec_;
};

#endif