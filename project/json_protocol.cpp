#include "json_protocol.hpp"

std::string LoginData(const std::string &username, const std::string &password)
{
    json j = {
        {"username", username},
        {"password", password}
    };
    return j.dump();
}

std::string makeRegisterData(const std::string& username, const std::string& password) 
{
    json j = {
        {"username", username},
        {"password", password}
    };
    return j.dump();
}

std::string makeAddFriendData(const std::string& username, const std::string& friendName) 
{
    json j = {
        {"username", username},
        {"friend", friendName}
    };
    return j.dump();
}

std::string makeFriendChatData(const std::string& from, const std::string& to, const std::string& message) 
{
    json j = {
        {"from", from},
        {"to", to},
        {"message", message}
    };
    return j.dump();
}

std::string makeGroupCreateData(const std::string& groupName, const std::string& creator) 
{
    json j = {
        {"group", groupName},
        {"creator", creator}
    };
    return j.dump();
}

std::string makeGroupJoinData(const std::string& groupName, const std::string& username) 
{
    json j = {
        {"group", groupName},
        {"username", username}
    };
    return j.dump();
}

std::string makeGroupChatData(const std::string& groupName, const std::string& from, const std::string& message)
{
    json j = {
        {"group", groupName},
        {"from", from},
        {"message", message}
    };
    return j.dump();
}

std::string ExtractCommonField(const json& j, const std::string& key)
{
    if (j.contains(key) && j[key].is_string())
        return j[key].get<std::string>();
    
    return "";
}