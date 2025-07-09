#include "Service.h"
#include "../muduo/logging/Logging.h"
#include <memory>
#include <mysql/mariadb_com.h>
#include <unistd.h>

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

std::string Escape(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '\'' || c == '\"' || c == '\\') output += '\\';
        output += c;
    }
    return output;
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

void Service::InitIdsFromMySQL() {
    ReadFromDataBase("select max(id) as id from users;", 
        [this](MysqlRow& userRow) {
            int maxUserId = userRow.GetInt("id");

            ReadFromDataBase("select max(id) as id from messages;", 
                [this, maxUserId](MysqlRow& msgRow) {
                    int maxMsgId = msgRow.GetInt("id");

                    gen_.InitUserId(maxUserId);
                    gen_.InitMsgId(maxMsgId);

                    LOG_INFO << "已从 MySQL 初始化 Redis ID:user=" << maxUserId << ", msg=" << maxMsgId;
                }
            );
        }
    );
}

void Service::ReadUserFromDB()
{
    std::string query = "select * from users;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int id = row.GetInt("id");
        std::string email = row.GetString("email");
        std::string username = row.GetString("name");
        std::string password = row.GetString("password");
        usermanager_.AddUser(id, username, password, email);
    });
}

void Service::ReadMessageFromDB()
{
    std::string query = "select * from messages;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int id = row.GetInt("id");
        int senderid = row.GetInt("senderid");
        int receiverid = row.GetInt("receiverid");
        std::string content = row.GetString("email");
        std::string type = row.GetString("type");
        std::string status = row.GetString("status");
        std::string time = row.GetString("timestamp");
        Message message(id, senderid, receiverid, content, type, status, time);
        messagemanager_.AddMessage(message);
    });
}

void Service::StartAutoFlushToDataBase(int seconds)
{
    running_ = true;
    flush_thread_ = std::thread([this, seconds]() {
        while (running_)
        {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
            FlushToDataBase();
        }
    });
}

void Service::FlushToDataBase()
{
    for (auto it : usermanager_.GetAllUser())
    {
        FormatUpdateUser(it.second);
    }

    for (auto it : messagemanager_.GetAllMessage())
    {
        FormatUpdateMessage(it.second);
    }
}

std::string Service::FormatUpdateUser(std::shared_ptr<User> user)
{
    std::ostringstream oss;
    oss << "replace into user (id, username, password, email) values (" 
        << user->GetId() << ", "<< Escape(user->GetName()) << ", "
        << Escape(user->GetPassword()) << ", " << Escape(user->GetEmail()) << ") ;";
    return oss.str();
}

std::string Service::FormatUpdateMessage(std::shared_ptr<Message> message)
{
    std::ostringstream oss;
    oss << "replace into message (id, senderid, receiverid, connect, type, status, timestamp) values ("
        << message->GetId() << message->GetSenderId() << message->GetReveiverId() 
        << Escape(message->GetContent()) << Escape(message->GetType())
        << Escape(message->GetStatus()) << Escape(message->GetTime()) << ") ;";
    return oss.str();
}

void Service::StopAutoFlush()
{
    if (flush_thread_.joinable())
        flush_thread_.join();
}

Service::Service()
{
    InitIdsFromMySQL();
    ReadUserFromDB();
    ReadMessageFromDB();

    StartAutoFlushToDataBase(10);
}

Service::~Service()
{
    FlushToDataBase();

    StopAutoFlush();
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

    std::cout << username << userid << password << email << std::endl;

    bool end = usermanager_.AddUser(userid, username, password, email);
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

