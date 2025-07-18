#include "json_protocol.hpp"
#include "entity/other.h"
#include "muduo/logging/Logging.h"
#include <optional>
#include <unordered_map>

//database

// chatconnect 实现
std::string js_data_chatconnect(const int& id, const std::string& type, const int& peerid) {
    json j = {
        {"id", id},
        {"type", type},
        {"peerid", peerid}
    };
    return j.dump();
}

// friend 实现
std::string js_data_friend(const int& id, const int& userid, const int& friendid, const std::string& status) {
    json j = {
        {"id", id},
        {"userid", userid},
        {"friendid", friendid},
        {"status", status}
    };
    return j.dump();
}

// groupapply 实现
std::string js_data_groupapply(const int& id, const int& groupid, const int& applyid) {
    json j = {
        {"id", id},
        {"groupid", groupid},
        {"applyid", applyid}
    };
    return j.dump();
}

// groups 实现
std::string js_data_groups(const int& id, const std::string& groupname) {
    json j = {
        {"id", id},
        {"groupname", groupname}
    };
    return j.dump();
}

// groupuser 实现
std::string js_data_groupuser(const int& id, const int& groupid, std::string& level) {
    json j = {
        {"id", id},
        {"groupid", groupid},
        {"level", level}
    };
    return j.dump();
}

// message 实现
std::string js_data_message(const int& id, const std::string& type, const int& senderid, 
                          const int& receiverid, const std::string& text, const std::string& status) {
    json j = {
        {"id", id},
        {"type", type},
        {"senderid", senderid},
        {"receiverid", receiverid},
        {"text", text},
        {"status", status}
    };
    return j.dump();
}

// user 实现
std::string js_data_user(const int& id, const std::string& username, 
                        const std::string password, bool online) {
    json j = {
        {"id", id},
        {"username", username},
        {"password", password},
        {"online", online}
    };
    return j.dump();
}

// userapply 实现
std::string js_data_userapply(const int& id, const int& userid, const int& applyid) {
    json j = {
        {"id", id},
        {"userid", userid},
        {"applyid", applyid}
    };
    return j.dump();
}

// 发送消息
std::string js_SendMessage(const std::string type, const int& senderid, const int& receiverid, const std::string text)
{
    json j = {
        {"type", type},
        {"userid", senderid},
        {"receiverid", receiverid},
        {"text", text}
    };
    return j.dump();
}

// 消息
std::string js_Message(const std::string& type, const int& senderid, const int& receiverid, const std::string& text, const std::string& status, std::string time)
{
    json j = {
        {"type", type},
        {"senderid", senderid},
        {"receiverid", receiverid},
        {"text", text},
        {"status", status},
        {"time", time}
    };
    return j.dump();
}

// 用户数据
std::string js_UserData(const int& userid, const std::string& username, const int& friendnum, const int& groupnum)
{
    json j = {
        {"userid", userid},
        {"username", username},
        {"friendnum", friendnum},
        {"groupnum", groupnum},
    };
    return j.dump();
}

// 注册
std::string js_RegisterData(const std::string& username, const std::string& password)
{
    json j = {
        {"username", username},
        {"password", password}
    };
    return j.dump();
}

// 用户id
std::string js_UserId(const int &userid)
{
    json j = {
        {"userid", userid}
    };
    return j.dump();
}

// 用户
std::string js_User(const int& userid, const std::string& username)
{
    json j = {
        {"userid", userid},
        {"username", username}
    };
    return j.dump();
}

// 登陆
std::string js_Login(const std::string username, const std::string password)
{
    json j = {
        {"username", username},
        {"password", password},
    };
    return j.dump();
}

// 登陆后数据包
std::string js_UserAllData(const int& userid, const std::unordered_map<int, Friend>& friends, const std::unordered_map<int, Group>& groups, const std::unordered_map<int, SimpUser>& friendapplylist)
{
    json j;
    j["userid"] = userid;
    j["friends"] = json::array();

    for (const auto& [id, f] : friends)
    {
        json jf;
        jf["friendid"] = f.GetFriendId();
        jf["friendname"] = f.GetFriendName();
        jf["status"] = f.GetStatus();
        j["friends"].push_back(jf);
    }

    j["groups"] = json::array();

    for (const auto& [id, g] : groups)
    {
        json jg;
        jg["groupid"] = g.GetGroupId();
        jg["groupname"] = g.GetGroupName();
        jg["members"] = json::array();

        for (const auto& [uid, user] : g.GetAllMembers())
        {
            json jm;
            jm["userid"] = user.GetUserId();
            jm["username"] = user.GetUserName();
            jm["role"] = user.GetRole();
            jm["muted"] = user.IsMuted();
            jg["members"].push_back(jm);
        }

        jg["apply"] = json::array();

        for (const auto& [uid, apply] : g.GetApplyList())
        {
            json ja;
            ja["applyid"] = apply.userid_;
            ja["applyname"] = apply.username_;
            jg["apply"].push_back(ja);
        }
        
        j["groups"].push_back(jg);
    }

    j["friendapplylist"] = json::array();

    for (auto& [uid, apply]: friendapplylist)
    {
        json ja;
        ja["applyid"] = apply.userid_;
        ja["applyname"] = apply.username_;
        j["friendapplylist"].push_back(ja);
    }
    
    return j.dump();
}

// 好友
std::string js_Friend(const int& friendid, const std::string& friendname, const bool online)
{
    json j = {
        {"friendid", friendid},
        {"friendname", friendname},
        {"online", online}
    };
    return j.dump();
}

// 好友列表
std::string js_FriendList(const int& userid, const std::unordered_map<int, bool>& friendlist)
{
    json j = {
        {"userid", userid},
        {"friendlist", friendlist}
    };
    return j.dump();
}

// 用户与好友
std::string js_UserWithFriend(const int& userid, const int& friendid)
{
    json j = {
        {"userid", userid},
        {"friendid", friendid}
    };
    return j.dump();
}

// 所有好友id与名字
std::string js_AllFriendIdName(const int& userid, const std::unordered_map<int, std::string>& friendidname, const std::unordered_map<int, std::string>& groupidname)
{
    json j = {
        {"userid", userid},
        {"friendidname", friendidname},
        {"groupidname", groupidname}
    };
    return j.dump();
}

// 群聊
std::string js_Group(const int& groupid, const std::string& groupname)
{
    json j = {
        {"groupid", groupid},
        {"groupname", groupname}
    };
    return j.dump();
}

// 用户与群聊id
std::string js_UserWithGroup(const int &userid, const int &groupid)
{
    json j = {
        {"userid", userid},
        {"groupid", groupid}
    };
    return j.dump();
}

// 群聊id
std::string js_GroupId(const int &groupid)
{
    json j = {
        {"groupid", groupid}
    };
    return j.dump();
}

// 群聊列表
std::string js_GroupList(const int& userid, const std::vector<int>& grouplist)
{
    json j = {
        {"userid", userid},
        {"grouplist", grouplist}
    };
    return j.dump();
}

// 创建群聊
std::string js_GroupCreateData(const std::string groupname, const int& creator, const std::vector<int>& othermembers)
{
    json j = {
        {"groupname", groupname},
        {"creator", creator},
        {"othermembers", othermembers}
    };
    return j.dump();
}

// 群聊成员
std::string js_GroupData(const int& groupid, const int& userid)
{
    json j = {
        {"groupid", groupid},
        {"userid", userid}
    };
    return j.dump();
}

// 群聊成员列表
std::string js_GroupMemberList(const int& groupid, const std::vector<int>& memberlist)
{
    json j = {
        {"groupid", groupid},
        {"memberlist", memberlist}
    };
    return j.dump();
}

std::string js_RefrushGroupCreate(const int& groupid, const std::string groupname, const std::unordered_map<int, GroupUser>& othermembers)
{
    json j;

    j["groupid"] = groupid;
    j["groupname"] = groupname;
    j["members"] = json::array();

    for (auto& [id, user] : othermembers)
    {
        json jm;
        jm["userid"] = user.GetUserId();
        jm["username"] = user.GetUserName();
        jm["role"] = user.GetRole();
        jm["muted"] = user.IsMuted();
        j["members"].push_back(jm);
    };
    return j.dump();
}

std::string js_Apply(const int& userid, const int& applyid, const std::string& applyname)
{
    json j = {
        {"userid", userid},
        {"applyid", applyid},
        {"applyname", applyname}
    };
    return j.dump();
}

std::string js_UserInterface(const int& userid, const int& peeid, const std::string& type)
{
    json j = {
        {"userid", userid},
        {"peeid", peeid},
        {"type", type}
    };
    return j.dump();
}


// 用于申请列表
std::string js_UserList(const std::vector<int>& userlist)
{
    json j = {
        {"userlist", userlist}
    };
    return j.dump();
}

std::string js_UserApply(const int& userid, const int& applicantid, bool result)
{
    json j = {
        {"userid", userid},
        {"applicantid", applicantid},
        {"result", result}
    };
    return j.dump();
}

std::string js_ApplyResult(const int& userid, const int& applicantid, const std::string applicantname, bool result)
{
    json j = {
        {"userid", userid},
        {"applicantid", applicantid},
        {"applicantname", applicantname},
        {"result", result}
    };
    return j.dump();
}

// 回复
std::string js_CommandReplyWithData(const std::string& json_str, const bool& end, const std::string& result)
{
    json j = {
        {"json_dump", json_str},
        {"end", end},
        {"result", result}
    };
    return j.dump();
}

std::string js_CommandReply(const bool& end, const std::string& result)
{
     json j = {
        {"end", end},
        {"result", result}
    };
    return j.dump();
}
