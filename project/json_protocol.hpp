#ifndef JSON_HPP
#define JSON_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;

//用户信息
std::string UserData(const std::string& username, const std::string& password);
//用户
std::string User(const std::string& username);

//好友列表
std::string FriendList(const std::string& usrname, const std::vector<std::string>& friendlist);
//用户与好友
std::string FriendData(const std::string& username, const std::string& friendname);
//私聊
std::string FriendChatData(const std::string& from, const std::string& to, const std::string& message);

//群聊列表
std::string GroupList(const std::string& username, const std::vector<std::string>& grouplist);
//创建群聊
std::string GroupCreateData(const std::string& groupname, const std::string& creator, const std::vector<std::string>& othermembers);
//群聊成员
std::string GroupData(const std::string& groupname, const std::string& username);
//群聊成员列表
std::string GroupMemberList(const std::string& groupname, const std::vector<std::string> &memberlist);
//群聊消息
std::string GroupChatData(const std::string& groupname, const std::string& from, const std::string& message);

//用于申请列表
std::string UserList(const std::vector<std::string>& userlist);

//回复
std::string CommandReply(const bool& end, const std::string& result);

//提取json变量
template<typename T>
std::optional<T> ExtractCommonField(const json& j, const std::string& key);

//更完备的提取json变量
template<typename T>
bool AssignIfPresent(const json& j, const std::string& key, T& out);

#endif