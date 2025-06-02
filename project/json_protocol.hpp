#ifndef JSON_HPP
#define JSON_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
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

//消息
std::string js_Message(const std::string& type, const int& senderid, const int& receiverid, const std::string& text, const std::string& status);

//打印用户信息
std::string js_UserData(const int& userid, const std::string& username, const int& friendnum, const int& groupnum);
//注册
std::string js_RegisterData(const std::string& username, const std::string& password);
//用户id
std::string js_UserId(const int& userid);
//用户
std::string js_User(const int& userid, const std::string& username);

//好友
std::string js_Friend(const int& friendid, const std::string& friendname, const bool online);
//好友列表
std::string js_FriendList(const int& userid, const std::unordered_map<int, bool>& friendlist);
//用户与好友
std::string js_FriendData(const int& userid, const int& friendid);
//所有好友群组id与名字
std::string js_AllFriendIdName(const int& userid, const std::unordered_map<int, std::string>& firendidname, const std::unordered_map<int, std::string>& groupidname);

//群聊
std::string js_Group(const int& groupid, const std::string& groupname);
//群聊id
std::string js_GroupId(const int& groupid);
//群聊列表
std::string js_GroupList(const int& userid, const std::vector<int>& grouplist);
//创建群聊
std::string js_GroupCreateData(const int& groupid, const std::string groupname, const int& creator, const std::vector<int>& othermembers);
//群聊成员
std::string js_GroupData(const int& groupid, const int& userid);
//群聊成员列表
std::string js_GroupMemberList(const int& groupid, const std::vector<int>& memberlist);

//用于申请列表
std::string js_UserList(const std::vector<int>& userlist);
//申请结果
std::string js_ApplyResult(const int& userid, const int& applicantid, bool result);

//回复
std::string js_CommandReply(const bool& end, const std::string& result);

//提取json变量
template<typename T>
std::optional<T> ExtractCommonField(const json& j, const std::string& key);

//更完备的提取json变量
template<typename T>
bool AssignIfPresent(const json& j, const std::string& key, T& out);

#endif