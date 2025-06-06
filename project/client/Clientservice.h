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
    void SendToFriend(const int& userid, const int& friendid, const std::string& message, const uint16_t seq);
    void SendToGroup(const int& userid, const int& groupid, const std::string& message, const uint16_t seq);

    void LoginRequest(const TcpConnectionPtr& conn,const std::string& username, const std::string& password, const uint16_t seq);
    void RegistrationRequest(const std::string& username, const std::string& password, const uint16_t seq);
    void GetPersonalChatHistory(const int& userid, const int& otherid, const uint16_t seq);
    void SendFriendRequest(const int& userid, const int& friendid, const uint16_t seq);
    void DeleteFriend(const int& userid, const int& friendid, const uint16_t seq);
    void GetGroupChatHistory(const int& userid, const int& groupid, const uint16_t seq);
    void ApplyToJoinGroupChat(const int& userid, const int& groupid, const uint16_t seq);
    void QuitGroup(const int& userid, const int& groupid, const uint16_t seq);
    void CreateGroup(const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const uint16_t seq);
    void BlockFriend(const int& userid, const int& friendid, const uint16_t seq);
    void ApprovalApply(const int& groupid, const int& applyid, const uint16_t seq);
    void RemoveGroupUser(const int& groupid, const int& userid, const uint16_t seq);
    void SetAdministrator(const int& groupid, const int& userid, const uint16_t seq);
    void RemoveAdministrator(const int& gorupid, const int& userid, const uint16_t seq);
    void DeleteGroup(const int& groupid, const uint16_t seq);

private:
    Codec codec_;
};

#endif