#include "Clientservice.h"
#include "Client.h"

void Clientservice::LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq)
{
    json packet  = js_Login(username, password);
    conn->send(codec_.encode(packet, 1, seq));
}

void Clientservice::RegistrationRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq)
{
    json packet = js_RegisterData(username, password);
    conn->send(codec_.encode(packet, 2, seq));
}

void Clientservice::GetPersonalChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& otherid, const uint16_t seq)
{
    json packet = js_FriendData(userid, otherid);
    conn->send(codec_.encode(packet, 21, seq));
}

void Clientservice::SendFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json packet = js_FriendData(userid, friendid);
    conn->send(codec_.encode(packet, 6, seq));
}

void Clientservice::DeleteFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json packet = js_FriendData(userid, friendid);
    conn->send(codec_.encode(packet, 4, seq));
}

void Clientservice::GetGroupChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq)
{
    json packet = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(packet, 22, seq));
}

void Clientservice::QuitGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq)
{
    json packet = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(packet, 15, seq));
}

void Clientservice::CreateGroup(const TcpConnectionPtr& conn, const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const uint16_t seq)
{
    json packet = js_GroupCreateData(groupname, userid, groupuserid);
    conn->send(codec_.encode(packet, 10, seq));
}

void Clientservice::BlockFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json packet = js_FriendData(userid, friendid);
    conn->send(codec_.encode(packet, 5, seq));
}

void Clientservice::RemoveGroupUser(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq)
{
    json packet = 
    conn->send(codec_.encode(packet, 2, seq));
}

void Clientservice::SetAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq)
{
    json packet = 
    conn->send(codec_.encode(packet, 2, seq));
}

void Clientservice::RemoveAdministrator(const TcpConnectionPtr& conn, const int& gorupid, const int& userid, const uint16_t seq)
{
    json packet = 
    conn->send(codec_.encode(packet, 2, seq));
}

void Clientservice::DeleteGroup(const TcpConnectionPtr& conn, const int& groupid, const uint16_t seq)
{
    json packet = 
    conn->send(codec_.encode(packet, 2, seq));
}