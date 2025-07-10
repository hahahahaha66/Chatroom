#include "Service.h"
#include "../muduo/logging/Logging.h"
#include <memory>
#include <mysql/mariadb_com.h>
#include <sw/redis++/shards.h>
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
    ReadFromDataBase("select max(id) as id from users;", [this](MysqlRow& userRow) 
    {
        int maxuserid = userRow.GetInt("id");
        gen_.InitUserId(maxuserid);
        LOG_INFO << "已从 MySQL 初始化 Redis ID:user=" << maxuserid;
    });

    ReadFromDataBase("select max(id) as id from messages;", [this](MysqlRow& msgRow) 
    {
        int maxmsgid = msgRow.GetInt("id");
        gen_.InitMsgId(maxmsgid);

        LOG_INFO << "已从 MySQL 初始化 Redis ID:msg=" << maxmsgid;
    });

    ReadFromDataBase("select max(id) as id from friends;", [this](MysqlRow& msgRow) 
    {
        int maxfriendid = msgRow.GetInt("id");
        gen_.InitFriendId(maxfriendid);

        LOG_INFO << "已从 MySQL 初始化 Redis ID:friend=" << maxfriendid;
    });

    ReadFromDataBase("select max(id) as id from friendapplys;", [this](MysqlRow& msgRow) 
    {
        int maxfriendapplyid = msgRow.GetInt("id");
        gen_.InitFriendAppltId(maxfriendapplyid);

        LOG_INFO << "已从 MySQL 初始化 Redis ID:friendapplys=" << maxfriendapplyid;
    });

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
        std::string content = row.GetString("content");
        std::string type = row.GetString("type");
        std::string status = row.GetString("status");
        std::string time = row.GetString("timestamp");
        Message message(id, senderid, receiverid, content, type, status, time);
        messagemanager_.AddMessage(message);
    });
}

void Service::ReadFriendFromDB()
{
    std::string query = "select * from friends;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int id = row.GetInt("id");
        int userid = row.GetInt("userid");
        int friendid = row.GetInt("friendid");
        bool block = row.GetBool("block");

        friendmanager_.AddFriend(id, userid, friendid, block);
    });
}

void Service::ReadFriendApplyFromDB()
{
    std::string query = "select * from friendapplys;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int applyid = row.GetInt("id");
        int fromid = row.GetInt("fromid");
        int targetid = row.GetInt("targetid");
        std::string status = row.GetString("status");

        friendapplymanager_.AddAplly(applyid, fromid, targetid, "Friend", status);
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
    databasethreadpool_.EnqueueTask([this](MysqlConnection& conn) {
        for (auto it : usermanager_.GetAllUser())
        {
            conn.ExcuteUpdata(FormatUpdateUser(it.second));
        }

        for (auto it : messagemanager_.GetAllMessage())
        {    
            conn.ExcuteUpdata(FormatUpdateMessage(it.second));
        }

        for (auto it : friendmanager_.GetAllFriend())
        {
            for (auto its : it.second)
            {
                conn.ExcuteUpdata(FormatUpdataFriend(its.second));
            }
        }

        for (auto it : friendapplymanager_.GetAllApply())
        {
            for (auto its : it.second)
            {
                conn.ExcuteUpdata(FormatUpdataFriendApply(its.second));
            }
        }
    });
}

std::string Service::FormatUpdateUser(std::shared_ptr<User> user)
{
    std::ostringstream oss;
    oss << "replace into users (id, name, password, email) values (" 
        << user->GetId() << ", '"<< Escape(user->GetName()) << "', '"
        << Escape(user->GetPassword()) << "', '" << Escape(user->GetEmail()) << "') ;";
    return oss.str();
}

std::string Service::FormatUpdateMessage(std::shared_ptr<Message> message)
{
    std::ostringstream oss;
    oss << "replace into messages (id, senderid, receiverid, content, type, status, timestamp) values ("
        << message->GetId() << ", '"<< message->GetSenderId() << "', "<< message->GetReveiverId() 
        << ", '"<< Escape(message->GetContent()) << "', '"<< Escape(message->GetType())
        << "', '"<< Escape(message->GetStatus()) << "', '"<< Escape(message->GetTime()) << "') ;";
    std::cout << oss.str() << std::endl;
    return oss.str();
}

std::string Service::FormatUpdataFriend(std::shared_ptr<Friend> frienda)
{
    std::ostringstream oss;
    oss << "replace into friends (id, userid, friendid, block) values (" 
        << frienda->GetId() << ", "<< frienda->GetUserId() << ", "
        << frienda->GetFriendId() << ", " << frienda->GetBlock() << ") ;";
    return oss.str();
}

std::string Service::FormatUpdataFriendApply(std::shared_ptr<Apply> friendapply)
{
    std::ostringstream oss;
    oss << "replace into friendapplys (id, fromid, targetid, status) values (" 
        << friendapply->GetApplyId() << ", "<< friendapply->GetFromId() << ", "
        << friendapply->GetTargetId() << ", '" << friendapply->GetStatus() << "') ;";
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
    ReadFriendFromDB();
    ReadFriendApplyFromDB();

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
        if (usermanager_.GetUser(it->second)->GetPassword() == password)
        {
            userid = it->second;
            end = true;
            usermanager_.SetOnline(userid);
            usermanager_.GetUser(userid)->SetConn(conn);
        }
    }

    json j = {
        {"end", end},
        {"id", userid}
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

    AssignIfPresent(js, "senderid", sendid);
    AssignIfPresent(js, "receiverid", receiverid);
    AssignIfPresent(js, "content", content);
    AssignIfPresent(js, "type", type);
    AssignIfPresent(js, "status", status);
    AssignIfPresent(js, "timestamp", timestamp);

    //添加到redis,获取自增id
    int msgid = gen_.GetNextMsgId();
    gen_.GetRedis()->set("global:msgid", std::to_string(msgid));

    json j;

    j = {
        {"end", true}
    };
    
    auto seconn = usermanager_.GetUser(sendid)->GetConn();
    seconn->send(code_.encode(j, "SendMessageBack"));

    if (type == "Group")
    {

    }
    else if (type == "Private")
    {
        if (usermanager_.IsOnline(receiverid))
        {
            status = "Read";
            auto reconn = usermanager_.GetUser(receiverid)->GetConn();
            j = {
                {"senderid", sendid},
                {"receiverid", receiverid},
                {"content", content},
                {"type", type},
                {"status", status},
                {"timestamp", timestamp}
            };
            reconn->send(code_.encode(j, "RecvMessage"));
        }
    }
    else
    {

    }
    Message message(msgid, sendid, receiverid, content, type, status, timestamp);
    messagemanager_.AddMessage(message);
}

void Service::AddFriend(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::ListFriendProceApplys(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::ListFriendUnproceApplys(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::ListFriendSentApplys(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::ProceFriendApplys(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::ListFriends(const TcpConnectionPtr& conn, const json& json, Timestamp)
{

}

void Service::GetChatHistory(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{

}

