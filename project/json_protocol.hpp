#ifndef JSON_HPP
#define JSON_HPP

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

std::string LoginData(const std::string& username, const std::string& password);
std::string RegisterData(const std::string username, const std::string& password);

std::string AddFriendData(const std::string& username, const std::string& friendname);
std::string FriendChatData(const std::string username, const std::string& to, const std::string& message);

std::string GroupCreateData(const std::string& groupname, const std::string& creator);
std::string GroupJoinData(const std::string& groupname, const std::string& username);
std::string GroupChatData(const std::string& groupname, const std::string& from, const std::string& message);

std::string ExtractCommonField(const json& j, const std::string& key);



std::string CommandReply(const bool& end, const std::string& result);


#endif