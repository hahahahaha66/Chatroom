#include "Service.h"
#include "../muduo/logging/Logging.h"

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

void Service::UserRegister(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    std::string username;
    std::string password;
    std::string email;

    AssignIfPresent(js, "username", username);
    AssignIfPresent(js, "password", password);
    AssignIfPresent(js, "email", email);

    bool end = usermanager_.addUserToSystem(username, password, email, conn);
    
    json j = {
        {"end", end}
    };

    conn->send(code_.encode(j, "RegisterBack"));
}

void Service::UserLogin(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    std::string password;
    std::string email;

    AssignIfPresent(js, "password", password);
    AssignIfPresent(js, "email", email);

    int userid = 0;
    bool end = false;

    auto emailtoid = usermanager_.GetEmailToId();
    auto it = emailtoid.find(email);

    if (it != emailtoid.end())
    {
        userid = it->second;
        end = true;
        usermanager_.SetOnline(userid);
    }

    json j = {
        {"end", end}
    };

    conn->send(code_.encode(j, "LoginBack"));
}

void Service::MessageSend(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    int sendid = 0;
    int receiverid = 0;
    std::string contect;
    std::string type;
    std::string status;

    AssignIfPresent(js, "sendid", sendid);
    AssignIfPresent(js, "recevierid", receiverid);
    AssignIfPresent(js, "contect", contect);
    AssignIfPresent(js, "type", type);
    AssignIfPresent(js, "status", status);

    //添加到redis,获取自增id
    
}

void Service::GetChatHistory(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{

}