#include "Service.h"
#include "../json_protocol.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

Service::Service(Dispatcher& dispatcher) : dispatcher_(dispatcher)
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
    handermap_[2] = std::bind(&Service::RegisterAccount, this, _1, _2, _3, _4);
    handermap_[5] = std::bind(&Service::ListFriendlist, this, _1, _2, _3, _4);
    handermap_[7] = std::bind(&Service::DeleteFriend, this, _1, _2, _3, _4);
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
}

void Service::RegisterAllHanders(Dispatcher& dispatcher)
{
    for (auto& [id, hander] : handermap_)
    {
        dispatcher.registerHander(id, hander);
    }
}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::unordered_map<int, std::string> friendlist;
    std::unordered_map<int, std::string> grouplist;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (end == true)
    {
        //这里执行从数据库中查询名字及密码是否正确
    }
    
    uint16_t type = (end == true ? 1 : 0);
    if (end) 
        userlist_[userid].SetOnline(conn);

    if (end)
    {
        for (auto& it : userlist_[userid].GetFriendList())
        {
            friendlist[it] = userlist_[it].GetUserName();
        }

        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist[it] = grouplist_[it].GetGroupName();
        }
    }
    
    json all_friend = js_AllFriendIdName(userid, friendlist, grouplist);
    conn->send(codec_.encode(all_friend, type, seq));
}


void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    if (end)
    {
        //检查数据库中有无已创建的账号，有返回false，没有则写入返回true
    }

    if (end)
        result = "Register successful!";
    else  
        result = "Register failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);
    if (end)
    {
        userlist_[userid] = {userid, username, password};
    }
       
    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendlist(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> friendset = userlist_[userid].GetFriendList();
    std::unordered_map<int, bool> friendlist;
    for (auto& it : friendset)
    {
        friendlist[it] = userlist_[it].IsOnLine();
    }

    json reply_js = js_FriendList(userid, friendlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    userlist_[userid].DeleteFriend(friendid);

    if (end)
        result = "Delete successful!";
    else  
        result = "Delete failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::BlockFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    userlist_[userid].AddBlockFriend(friendid);

    if (end)
        result = "Block successful!";
    else  
        result = "Block failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::AddFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int applicantid;  //发送好友申请
    int userid;  //接收好友申请
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", applicantid);

    userlist_[userid].AddApply(applicantid);

    if (end)
        result = "Add apply successful!";
    else  
        result = "Add apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> applyset = userlist_[userid].GetApplyList();
    std::unordered_map<int, bool> applylist;
    for (auto& it : applyset)
    {
        applylist[it] = userlist_[it].IsOnLine();
    }

    json reply_js = js_FriendList(userid, applylist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessFriendApply(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int applicantid;
    bool real_result;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "applicantid", applicantid);
    end &= AssignIfPresent(js, "result", real_result);

    userlist_[userid].DeleteApply(applicantid);
    if (real_result)
    {
        userlist_[userid].AddFriend(applicantid);
    }

    if (end)
        result = "Process apply successful!";
    else  
        result = "Process apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> groupset = userlist_[userid].GetGroupList();
    std::vector<int> grouplist;

    for (auto&it : groupset)
    {
        grouplist.push_back(it);
    }

    json reply_js = js_UserList(grouplist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::CreateGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    std::string groupname;
    int creatorid;
    std::vector<int> othermembers;
    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "groupname", groupname);
    end &= AssignIfPresent(js, "creatorid", creatorid);
    end &= AssignIfPresent(js, "othermember", othermembers);

    grouplist_[groupid] = {groupid, groupname};
    grouplist_[groupid].AddMember(std::move(GroupUser(creatorid, "Group_owner")));
    for (auto& it : othermembers)
    {
        grouplist_[groupid].AddMember(std::move(GroupUser(it, "Member")));
    }

    userlist_[creatorid].JoinGroup(groupid);
    for (auto& it : othermembers)
    {
        userlist_[it].JoinGroup(groupid);
    }
    
    if (end)
        result = "Create group successful!";
    else  
        result = "Create group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupMemberList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int groupid;
    end &= AssignIfPresent(js, "groupid", groupid);
    
    std::vector<int> memberlist;
    if (end) 
    {
        std::unordered_map<int, GroupUser> temp = grouplist_[groupid].GetAllMembers();
        for (auto& it : temp) 
        {
            memberlist.push_back(it.second.GetUserId());
        }
    }

    json reply_js = js_UserList(memberlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int groupid;
    end &= AssignIfPresent(js, "groupid", groupid);

    std::vector<int> applylist_;
    if (end)
    {
        for (auto& it : grouplist_[groupid].GetApplyList())
        {
            applylist_.push_back(it);
        }
    }
    
    json reply_js = js_UserList(applylist_);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessGroupApply(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    int applicantid;
    bool real_result;
    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "applicantid", applicantid);
    end &= AssignIfPresent(js, "result", real_result);

    grouplist_[groupid].DeleteApply(applicantid);
    if (real_result)
    {
        grouplist_[groupid].AddMember(std::move(GroupUser(applicantid, "Member")));
    }

    if (end)
        result = "Process apply successful!";
    else  
        result = "Process apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::QuitGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    userlist_[userid].LeaveGroup(groupid);
    grouplist_[groupid].RemoveMember(userid);

    if (end)
        result = "Quit group successful!";
    else  
        result = "Quit group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (grouplist_[groupid].GetMember(userid)->GetRole() == "Group_owner")
    {
        for (auto& it : grouplist_[groupid].GetAllMembers())
        {
            userlist_[it.first].LeaveGroup(groupid);
        }

        grouplist_.erase(groupid);
    }
    else 
    {
        end = false;
    }

    if (end)
        result = "Delete group successful!";
    else  
        result = "Delete group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::PrintUserData(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    
    int userid;
    end &= AssignIfPresent(js, "userid", userid);
    
    std::string username = userlist_[userid].GetUserName();
    int friendnum = userlist_[userid].GetFriendList().size();
    int groupnum = userlist_[userid].GetGroupList().size();

    json reply_js = js_UserData(userid, username, friendnum, groupnum);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ChangeUserPassword(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    userlist_[userid].SetPassWord(password);

    if (end)
        result = "Chance password successful!";
    else  
        result = "Chance password failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteUserAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (userlist_[userid].GetPassWord() == password)
    {
        for (auto& it : userlist_[userid].GetFriendList())
        {
            userlist_[it].DeleteFriend(userid);
        }
        
        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist_[it].RemoveMember(userid);
        }
        userlist_.erase(userid);
    }
    else {
        end = false;
    }

    if (end)
        result = "Delete account successful!";
    else  
        result = "Delete account failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}