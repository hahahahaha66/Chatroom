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

void Service::ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander)
{
    databasethreadpool_.EnqueueTask([this, query, rowhander](MysqlConnection& conn) {
        MYSQL_RES* res = conn.ExcuteQuery(query);
        if (!res) 
        {
            return;
        }

        MysqlResult result(res);
        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            rowhander(row);
        }
    });
}

Service::Service()
{
    int maxuserid = 0;
    int maxmsgid = 0;
    std::string query = "select max(id) from users;";
    ReadFromDataBase(query, [this, maxuserid](MysqlRow& row)mutable {
        maxuserid = row.GetInt("id");
    });
    query = "select max(id) from messages;";
    ReadFromDataBase(query, [this, maxmsgid](MysqlRow& row)mutable {
        maxmsgid = row.GetInt("id");
    });

    gen_.InitUserId(maxuserid);
    gen_.InitMsgId(maxmsgid);
}

void Service::UserRegister(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    std::string username;
    std::string password;
    std::string email;

    AssignIfPresent(js, "username", username);
    AssignIfPresent(js, "password", password);
    AssignIfPresent(js, "email", email);

    int userid = gen_.GetNextUserId();

    bool end = usermanager_.addUserToSystem(userid, username, password, email, conn);
    gen_.GetRedis()->set("global:userid", std::to_string(userid));
    
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
    std::string content;
    std::string type;
    std::string status;
    std::string timestamp;

    AssignIfPresent(js, "sendid", sendid);
    AssignIfPresent(js, "recevierid", receiverid);
    AssignIfPresent(js, "contect", content);
    AssignIfPresent(js, "type", type);
    AssignIfPresent(js, "status", status);
    AssignIfPresent(js, "timestamp", timestamp);

    //添加到redis,获取自增id
    int msgid = gen_.GetNextMsgId();
    gen_.GetRedis()->set("global:userid", std::to_string(msgid));

    if (type == "Group")
    {

    }
    else if (type == "Private")
    {
        if (usermanager_.IsOnline(receiverid))
        {
            status = "Read";
            auto reconn = usermanager_.GetUser(receiverid)->GetConn();
            json j = {
                {"sendid", sendid},
                {"recevierid", receiverid},
                {"contect", content},
                {"type", type},
                {"status", status},
                {"timestamp", timestamp}
            };

            conn->send(code_.encode(j, "Message"));
        }
    }
    else
    {

    }
    Message message(msgid, sendid, receiverid, content, type, status, timestamp);
    messagemanager_.AddMessage(message);
}

void Service::GetChatHistory(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{

}