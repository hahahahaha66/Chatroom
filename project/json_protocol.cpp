#include "json_protocol.hpp"
#include "muduo/logging/Logging.h"

std::string UserData(const std::string &username, const std::string &password)
{
    json j = {
        {"username", username},
        {"password", password}
    };
    return j.dump();
}

std::string FriendList(const std::string &usrname, const std::vector<std::string> &friendlist)
{
    json j = {
        {"username", usrname},
        {"friendlist", friendlist}
    };
    return j.dump();
}

std::string FriendData(const std::string& username, const std::string& friendname) 
{
    json j = {
        {"username", username},
        {"friendname", friendname}
    };
    return j.dump();
}

std::string FriendChatData(const std::string& from, const std::string& to, const std::string& message) 
{
    json j = {
        {"from", from},
        {"to", to},
        {"message", message}
    };
    return j.dump();
}

std::string GroupCreateData(const std::string& groupName, const std::string& creator, const std::vector<std::string>& othermembers) 
{
    json j = {
        {"groupname", groupName},
        {"creator", creator},
        {"othermembers", othermembers}
    };
    return j.dump();
}

std::string GroupData(const std::string& groupName, const std::string& username) 
{
    json j = {
        {"groupname", groupName},
        {"username", username}
    };
    return j.dump();
}

std::string GroupMemberList(const std::string &groupname, const std::vector<std::string>& memberlist)
{
    json j = {
        {"groupname", groupname},
        {"memberlist", memberlist}
    };
    return j.dump();
}

std::string GroupChatData(const std::string& groupname, const std::string& from, const std::string& message)
{
    json j = {
        {"groupname", groupname},
        {"from", from},
        {"message", message}
    };
    return j.dump();
}

std::string UserList(const std::vector<std::string>& userlist)
{
    json j = {
        {"userlist", userlist}
    };
    return j.dump();
}

std::string CommandReply(const bool &end, const std::string &result)
{
    json j = {
        {"end", end},
        {"result", result}
    };
    return j.dump();
}

template<typename  T>
std::string ExtractCommonField(const json& j, const std::string& key, const T& default_value)
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
    return default_value;
}