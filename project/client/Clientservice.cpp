#include "Clientservice.h"

#include <unordered_map>

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
    json j = js_UserApply(userid, friendid, result);
    conn->send(codec_.encode(j, 8, seq));
}
void Clientservice::ProcessingGroupRequest(const TcpConnectionPtr& conn, const int& groupid, const int& userid, bool result, const uint16_t seq)
{
    json j = js_UserApply(groupid, userid, result);
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

void Clientservice::UpdatedUserInterface(const TcpConnectionPtr& conn, int& peeid, std::string& type, const uint16_t seq)
{
    json j = js_UserInterface(userid_, peeid, type);
    conn->send(codec_.encode(j, 19, seq));
}



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
    bool end = true;

    end &= AssignIfPresent(js, "userid", userid_);

    int friendid;
    std::string friendname;
    std::string status;

    if (end)
    {
        for (const auto& jf : js["friends"])
        {
            end &= AssignIfPresent(jf, "friendid", friendid);
            end &= AssignIfPresent(jf, "friendname", friendname);
            end &= AssignIfPresent(jf, "status", status);

            Friend f(userid_, friendname, friendid, status);

            friendlist_[friendid] = f;
        }
    }

    int groupid;
    std::string groupname;

    if (end)
    {
        for (const auto& jg : js["groups"])
        {
            end &= AssignIfPresent(jg, "groupid", groupid);
            end &= AssignIfPresent(jg, "groupname", groupname);

            Group g(groupid, groupname);

            int uid;
            std::string username;
            std::string role;
            bool muted;
            for (const auto& jm : jg["members"])
            {
                end &= AssignIfPresent(jm, "uid", uid);
                end &= AssignIfPresent(jm, "username", username);
                end &= AssignIfPresent(jm, "role", role);
                end &= AssignIfPresent(jm, "muted", muted);

                GroupUser gu(uid, role);
                gu.SetMuted(muted);
                gu.SetUserName(username);

                g.AddMember(gu);
            }

            int applyid;
            std::string applyname;
            for (const auto& ja : jg["apply"])
            {
                end &= AssignIfPresent(ja, "applyid", applyid);
                end &= AssignIfPresent(ja, "applyname", applyname);

                g.AddApply(applyid, applyname);
            }

            grouplist_[groupid] = g;
        }
    }

    if (end)
    {
        for (auto& ja : js["friendapplylist"])
        {
            int applyid;
            std::string applyname;

            end &= AssignIfPresent(ja, "applyid", applyid);
            end &= AssignIfPresent(ja, "applyname", applyname);

            friendapplylist[applyid] = std::move(SimpUser(applyid, applyname));
        }
    }

    if (end)
    {
        std::cout << "Read user data successfully" << std::endl;
    }
    else  
    {
        std::cout << "Read user data failed" << std::endl;
    }
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
    std::string time_;

    for (const auto& it : js["message"])
    {
        AssignIfPresent(it, "senderid", senderid);
        AssignIfPresent(it, "connect", connect);
        AssignIfPresent(it, "time", time_);

        std::cout << time_ << " " << friendlist_[senderid].GetFriendName() << " : " << connect << std::endl;
    }
}
void Clientservice::Back_GetGroupChatHistory(const json& js, Timestamp time)
{
    int senderid;  
    int receiverid;
    std::string connect;
    std::string time_;

    for (const auto& it : js["message"])
    {
        AssignIfPresent(it, "senderid", senderid);
        AssignIfPresent(it, "connect", connect);
        AssignIfPresent(it, "time", time_);
        AssignIfPresent(it, "receiverid", receiverid);

        std::cout << time_ << " " << grouplist_[receiverid].GetMember(senderid)->GetUserName() << " : " << connect << std::endl;
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
    bool end = true;
    std::string json_dump;
    end &= AssignIfPresent(js, "json_dump", json_dump);
    json data = json::parse(json_dump);

    int userid;
    int applicantid;
    std::string applicantname;
    bool result;
    end &= AssignIfPresent(data, "userid", userid);
    end &= AssignIfPresent(data, "applicantid", applicantid);
    end &= AssignIfPresent(data, "applicantname", applicantname);
    end &= AssignIfPresent(data, "result", result);

    if (CommandReply(js, time) && userid ==userid_ && end)
    {
        friendapplylist.erase(applicantid);
        if (result)
        {
            friendlist_[applicantid] = std::move(Friend(userid, applicantid, applicantname));
        }            
    }
    else
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
void Clientservice::ProcessingGroupRequest(const json& js, Timestamp time)
{
    CommandReply(js, time);
}

//好友相关回调
void Clientservice::Back_BlockFriend(const json& js, Timestamp time)
{
    bool end = true;
    std::string json_dump;
    end &= AssignIfPresent(js, "json_dump", json_dump);
    json data = json::parse(json_dump);

    int userid;
    int friendid;
    end &= AssignIfPresent(data, "userid", userid);
    end &= AssignIfPresent(data, "friendid", friendid);

    if (CommandReply(js, time) && userid ==userid_ && end)
    {
        if (friendlist_[friendid].GetBlocked())
        {
            friendlist_[friendid].SetBlocked(false);
        }
        else  
        {
            friendlist_[friendid].SetBlocked(true);
        }
    }
    else
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
void Clientservice::Back_DeleteFriend(const json& js, Timestamp time)
{
    bool end = true;
    std::string json_dump;
    end &= AssignIfPresent(js, "json_dump", json_dump);
    json data = json::parse(json_dump);

    int userid;
    int friendid;
    end &= AssignIfPresent(data, "userid", userid);
    end &= AssignIfPresent(data, "friendid", friendid);

    if (CommandReply(js, time) && userid ==userid_ && end)
    {
        friendlist_.erase(userid);
    }
    else
    {
        std::cout << "Wrong happened" << std::endl;
    }
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

bool Clientservice::CommandReply(const json& js, Timestamp time)
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
    else  
    {
        std::cout << "解析失败" << std::endl;
    }

    return state && end;
}

void Clientservice::RefreshMessage(const json& js, Timestamp time)
{
    bool end = true;

    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    std::string status;
    std::string time_;
    end &= AssignIfPresent(js, "type", type);
    end &= AssignIfPresent(js, "senderid", senderid);
    end &= AssignIfPresent(js, "receiverid", receiverid);
    end &= AssignIfPresent(js, "connect", connect);
    end &= AssignIfPresent(js, "status", status);
    end &= AssignIfPresent(js, "time", time_);

    if (end)
    {
        if (type == "Private")
        {
            std::cout << time_ << " " << friendlist_[senderid].GetFriendName() << " : " << connect << std::endl;
        }
        else if (type == "Group")
        {
            std::cout << time_ << " " << grouplist_[receiverid].GetMember(senderid)->GetUserName() << " : " << connect << std::endl;
        }
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshFriendDelete(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    if (end)
    {
        friendlist_.erase(userid);
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshFriendBlock(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    if (end)
    {
        friendlist_[userid].SetBlocked(true);
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshFriendAddApply(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int friendid;
    std::string friendname;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "applyid", friendid);
    end &= AssignIfPresent(js, "applyname", friendname);
    if (end)
    {
        if (userid == userid_)
        {
            friendapplylist[friendid] = std::move(SimpUser(friendid, friendname));
        }
        else  
        {
            std::cout << "Wrong happened" << std::endl;
        }
    }
    else  
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshGroupAddApply(const json& js, Timestamp time)
{
    bool end = true;

    int groupid;
    int userid;
    std::string username;

    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "applyid", userid);
    end &= AssignIfPresent(js, "applyname", username);

    if (end)
    {
        grouplist_[groupid].AddApply(userid, username);
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshGroupCreate(const json& js, Timestamp time)
{
    bool end = true;

    int groupid;
    std::string groupname;

    end &= AssignIfPresent(js, "groupid", groupid);
    end &= AssignIfPresent(js, "groupname", groupname);

    if (end)
        grouplist_[groupid] = std::move(Group(groupid, groupname));
    else  
        std::cout << "Wrong happened" << std::endl;

    int userid; 
    std::string username;
    std::string userrole;
    bool usermuted;
    for (auto& it : js["members"])
    {
        end &= AssignIfPresent(it, "userid", userid);
        end &= AssignIfPresent(it, "username", username);
        end &= AssignIfPresent(it, "role", userrole);
        end &= AssignIfPresent(it, "muted", usermuted);

        if (end)
        {
            grouplist_[groupid].AddMember(std::move(GroupUser(userid, userrole)));
            grouplist_[groupid].GetMember(userid)->SetUserName(username);
            grouplist_[groupid].GetMember(userid)->SetMuted(usermuted);
        }
        else 
        {
            std::cout << "Wrong happened" << std::endl;
        }
        
    }

}

void Clientservice::RefreshFriendApplyProcess(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int applyid;
    std::string applyname;
    bool result;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "applicantid", applyid);
    end &= AssignIfPresent(js, "applicantname", applyname);
    end &= AssignIfPresent(js, "result", result);

    if (end)
    {
        if (userid == userid_)
        {
            friendapplylist.erase(applyid);
            if (result)
            {
                friendlist_[applyid] = std::move(Friend(userid, applyid, applyname));
            }
        }
        
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}

void Clientservice::RefreshGroupApplyProcess(const json& js, Timestamp time)
{
    bool end = true;

    int groupid;
    int applyid;
    std::string applyname;
    bool result;

    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "applicantid", applyid);
    end &= AssignIfPresent(js, "applicantname", applyname);
    end &= AssignIfPresent(js, "result", result);

    if (end)
    {
        if (result)
        {
            grouplist_[groupid].ApprovalApply(applyid);
            grouplist_[groupid].GetMember(applyid)->SetUserName(applyname);
        }
        else  
        {
            grouplist_[groupid].DeleteApply(applyid);
        }
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
    
void Clientservice::RefreshGroupQuitUser(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (end)
    {
        if (userid == userid_)
        {
            grouplist_.erase(groupid);
        }
        else  
        {
            grouplist_[groupid].RemoveMember(userid);
        }
    }
    else 
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
    
void Clientservice::RefreshGroupRemoveUser(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (end)
    {
        if (userid == userid_)
        {
            grouplist_.erase(groupid);
        }
        else  
        {
            grouplist_[groupid].RemoveMember(userid);
        }
    }
    else  
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
    
void Clientservice::RefreshGroupAddAdministrator(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (end)
    {
        grouplist_[groupid].GetMember(userid)->SetRole("Administrator");
    }
    else  
    {
        std::cout << "Wrong happened" << std::endl;
    }
}
    
void Clientservice::RefreshGroupRemoveAdministrator(const json& js, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (end)
    {
        grouplist_[groupid].GetMember(userid)->SetRole("Member");
    }
    else  
    {
        std::cout << "Wrong happened" << std::endl;
    }
}