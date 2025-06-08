#include "Clientservice.h"

//消息
void Clientservice::SendToFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const std::string& message, const uint16_t seq)
{
    json j = js_SendMessage("Private", userid, friendid, message);
    conn->send(codec_.encode(j, 14, seq));
}
void Clientservice::SendToGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const std::string& message, const uint16_t seq)
{
    json j = js_SendMessage("Group", userid, groupid, message);
    conn->send(codec_.encode(j, 14, seq));
}

//登陆注册
void Clientservice::LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq)
{
    json j = js_Login(username, password);
    conn->send(codec_.encode(j, 1, seq));
}
void Clientservice::RegistrationRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq)
{
    json j = js_RegisterData(username, password);
    conn->send(codec_.encode(j, 2, seq));
}

//获取聊天历史
void Clientservice::GetPersonalChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json j = js_UserWithFriend(userid, friendid);
    conn->send(codec_.encode(j, 15, seq));
}
void Clientservice::GetGroupChatHistory(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j, 16, seq));
}

//发送加好友和加群请求
void Clientservice::SendFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json j = js_UserWithFriend(userid, friendid);
    conn->send(codec_.encode(j, 5, seq));
}
void Clientservice::SendGroupRequest(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j, 6, seq));
}

//处理加好友和加群请求
void Clientservice::ProcessingFriendRequest(const TcpConnectionPtr& conn, const int& userid, const int& friendid, bool result, const uint16_t seq)
{
    json j = js_ApplyResult(userid, friendid, result);
    conn->send(codec_.encode(j, 8, seq));
}
void Clientservice::ProcessingGroupRequest(const TcpConnectionPtr& conn, const int& groupid, const int& userid, bool result, const uint16_t seq)
{
    json j = js_ApplyResult(groupid, userid, result);
    conn->send(codec_.encode(j, 9, seq));
}

//朋友操作
void Clientservice::DeleteFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json j = js_UserWithFriend(userid, friendid);
    conn->send(codec_.encode(j, 3, seq));
}
void Clientservice::BlockFriend(const TcpConnectionPtr& conn, const int& userid, const int& friendid, const uint16_t seq)
{
    json j = js_UserWithFriend(userid, friendid);
    conn->send(codec_.encode(j, 4, seq));
}

//群聊操作
void Clientservice::QuitGroup(const TcpConnectionPtr& conn, const int& userid, const int& groupid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j, 10, seq));
}
void Clientservice::CreateGroup(const TcpConnectionPtr& conn, const int& userid, const std::string groupname, const std::vector<int>& groupuserid, const uint16_t seq)
{
    json j = js_GroupCreateData(groupname, userid, groupuserid);
    conn->send(codec_.encode(j, 7, seq));
}
void Clientservice::RemoveGroupUser(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j, 11, seq));
}
void Clientservice::SetAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j,17 , seq));
}
void Clientservice::RemoveAdministrator(const TcpConnectionPtr& conn, const int& groupid, const int& userid, const uint16_t seq)
{
    json j = js_UserWithGroup(userid, groupid);
    conn->send(codec_.encode(j, 18, seq));
}
// void Clientservice::DeleteGroup(const TcpConnectionPtr& conn, const int& groupid, const uint16_t seq)
// {
//     json j = js;
//     conn->send(codec_.encode(j, 14, seq));
// }



//消息回调
void Clientservice::Back_SendToFriend(const json& js, Timestamp time)
{
    bool state = true;

    bool end;
    std::string result;
    state &= AssignIfPresent(js, "end", end);
    state &= AssignIfPresent(js, "result", result);

    if (state)
    {

    }
    else  
    {
        std::cout << result << std::endl;
    }
}
void Clientservice::Back_SendToGroup(const json& js, Timestamp time)
{
    bool state = true;

    bool end;
    std::string result;
    state &= AssignIfPresent(js, "end", end);
    state &= AssignIfPresent(js, "result", result);

    if (state)
    {

    }
    else  
    {
        std::cout << result << std::endl;
    }
}

//登陆注册回调
void Clientservice::Back_LoginRequest(const json& js, Timestamp time)
{

}
void Clientservice::Back_RegistrationRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}

//获取聊天历史回调
void Clientservice::Back_GetPersonalChatHistory(const json& js, Timestamp time)
{
    int senderid;  
    std::string connect;

    for (const auto& it : js["message"])
    {
        AssignIfPresent(it, "senderid", senderid);
        AssignIfPresent(it, "connect", connect);

        std::cout << friendlist_[senderid].GetFriendName() << " : " << connect << std::endl;
    }
}
void Clientservice::Back_GetGroupChatHistory(const json& js, Timestamp time)
{
    int senderid;  
    std::string connect;

    for (const auto& it : js["message"])
    {
        AssignIfPresent(it, "senderid", senderid);
        AssignIfPresent(it, "connect", connect);

        std::cout << friendlist_[senderid].GetFriendName() << " : " << connect << std::endl;
    }
}

//好友与群聊请求回调
void Clientservice::Back_SendFriendRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_SendGroupRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}

//处理申请回调
void Clientservice::ProcessingFriendRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::ProcessingGroupRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}

//好友相关回调
void Clientservice::Back_BlockFriend(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_DeleteFriend(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
    
//群聊相关回调
void Clientservice::Back_QuitGroup(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_CreateGroup(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_RemoveGroupUser(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_SetAdministrator(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_RemoveAdministrator(const json& js, Timestamp time)
{
    CommandReply(js, time);
}
void Clientservice::Back_DeleteGroup(const json& js, Timestamp time)
{

}

void Clientservice::CommandReply(const json& js, Timestamp time)
{
    bool state = true;

    bool end;
    std::string result;
    state &= AssignIfPresent(js, "end", end);
    state &= AssignIfPresent(js, "result", result);

    if (state)
    {
        std::cout << result << std::endl;
    }
}