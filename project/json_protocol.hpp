#ifndef JSON_HPP
#define JSON_HPP

#include "entity/Friend.h"
#include "entity/Group.h"
#include "entity/GroupUser.h"
#include "entity/other.h"
#include "muduo/logging/Logging.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

//database

//chatconect
std::string js_data_chatconnect(const int& id, const std::string& type, const int&peerid);
//friend
std::string js_data_friend(const int& id, const int& userid, const int& friendid, const std::string& status);
//groupapply
std::string js_data_groupapply(const int& id, const int& groupid, const int& applyid);
//groups
std::string js_data_groups(const int& id, const std::string& groupname);
//groupuser
std::string js_data_groupuser(const int& id, const int& groupid, std::string& level);
//message
std::string js_data_message(const int& id, const std::string& type, const int& senderid, const int& reveiverid, const std::string& text, const std::string& status);
//user
std::string js_data_user(const int& id, const std::string& username, const std::string password, bool online);
//userapply
std::string js_data_userapply(const int& id, const int& userid, const int& applyid);

//发送消息
std::string js_SendMessage(const std::string type, const int& userid, const int& friendid, const std::string text);
//存储消息
std::string js_Message(const std::string& type, const int& senderid, const int& receiverid, const std::string& text, const std::string& status, std::string time);

//打印用户信息
std::string js_UserData(const int& userid, const std::string& username, const int& friendnum, const int& groupnum);
//注册
std::string js_RegisterData(const std::string& username, const std::string& password);
//用户id
std::string js_UserId(const int& userid);
//登陆
std::string js_Login(const std::string username, const std::string password);
//登陆后放在客户端缓存的数据包
std::string js_UserAllData(const int& userid, const std::unordered_map<int, Friend>& friends, const std::unordered_map<int, Group>& groups, const std::unordered_map<int, SimpUser>& friendapplyllist);
//用户
std::string js_User(const int& userid, const std::string& username);

//好友
std::string js_Friend(const int& friendid, const std::string& friendname, const bool online);
//好友列表
std::string js_FriendList(const int& userid, const std::unordered_map<int, bool>& friendlist);
//用户与好友
std::string js_UserWithFriend(const int& userid, const int& friendid);
//所有好友群组id与名字
std::string js_AllFriendIdName(const int& userid, const std::unordered_map<int, std::string>& firendidname, const std::unordered_map<int, std::string>& groupidname);

//群聊
std::string js_Group(const int& groupid, const std::string& groupname);
//用户与群聊 
std::string js_UserWithGroup(const int& userid, const int& groupid);
//群聊id
std::string js_GroupId(const int& groupid);
//群聊列表
std::string js_GroupList(const int& userid, const std::vector<int>& grouplist);
//创建群聊
std::string js_GroupCreateData(const std::string groupname, const int& creator, const std::vector<int>& othermembers);
//群聊成员
std::string js_GroupData(const int& groupid, const int& userid);
//群聊成员列表
std::string js_GroupMemberList(const int& groupid, const std::vector<int>& memberlist);
//刷新群聊创建
std::string js_RefrushGroupCreate(const int& groupid, const std::string groupname, const std::unordered_map<int, GroupUser>& othermembers);

//通用申请
std::string js_Apply(const int& userid, const int& applyid, const std::string& applyname);

//更新用户界面信息
std::string js_UserInterface(const int& userid, const int& peeid, const std::string& type);

//用于申请列表
std::string js_UserList(const std::vector<int>& userlist);
//用户申请
std::string js_UserApply(const int& userid, const int& applicantid, bool result);
//申请结果
std::string js_ApplyResult(const int& userid, const int& applicantid, const std::string applicantname, bool result);

//回复
std::string js_CommandReplyWithData(const std::string& json_str, const bool& end, const std::string& result);
std::string js_CommandReply(const bool& end, const std::string& result);

//提取json变量
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
        LOG_ERROR << "Extract field [" << key << "] error: " << e.what();
        // 也可以记录当前的json值
        LOG_ERROR << "Json value for key [" << key << "] is: " << j.at(key).dump();
    }
    return std::nullopt;
}

//更完备的提取json变量
template<typename T>
bool AssignIfPresent(const json& j, const std::string& key, T& out)
{
    if (j.is_string())
    {
        try 
        {
            json pared = json::parse(j.get<std::string>());
            return AssignIfPresent(pared, key, out);
        }
        catch(...)
        {
            return false;
        }
    }


    auto opt = ExtractCommonField<T>(j, key);
    if (opt.has_value()) {
        out = std::move(opt.value());
        return true;
    }
    return false;
}


#endif