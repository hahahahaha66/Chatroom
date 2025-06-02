#include "json_protocol.hpp"
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

std::string js_Message(const std::string& type, const int& senderid, const int& receiverid, const std::string& text, const std::string& status)
{
    json j = {
        {"type", type},
        {"senderid", senderid},
        {"receiverid", receiverid},
        {"text", text},
        {"status", status}
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
std::string js_FriendData(const int& userid, const int& friendid)
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
std::string js_GroupCreateData(const int& groupid, const std::string groupname, const int& creator, const std::vector<int>& othermembers)
{
    json j = {
        {"groupid", groupid},
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

// 用于申请列表
std::string js_UserList(const std::vector<int>& userlist)
{
    json j = {
        {"userlist", userlist}
    };
    return j.dump();
}

std::string js_ApplyResult(const int& userid, const int& applicantid, bool result)
{
    json j = {
        {"userid", userid},
        {"applicantid", applicantid},
        {"result", result}
    };
    return j.dump();
}

// 回复
std::string js_CommandReply(const bool& end, const std::string& result)
{
    json j = {
        {"end", end},
        {"result", result}
    };
    return j.dump();
}


template<typename  T>
std::optional<T> ExtractCommonField(const json& j, const std::string& key)
{
    try 
    {
        if (j.contains(key) && !j.at(key).is_null())
        {
            return j.at(key).get<T>();
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << e.what();
    }
    return std::nullopt;
}

template<typename T>
bool AssignIfPresent(const json& j, const std::string& key, T& out)
{
    auto opt = ExtractCommonField<T>(j, key);
    if (opt.has_value()) {
        out = std::move(opt.value());
        return true;
    }
    return false;
}