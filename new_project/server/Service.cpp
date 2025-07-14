#include "Service.h"
#include "../muduo/logging/Logging.h"
#include <exception>
#include <memory>
#include <mysql/mariadb_com.h>
#include <mysql/mysql.h>
#include <ostream>
#include <sstream>
#include <string>
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

std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;

    // 线程安全地转换为本地时间
#if defined(_WIN32)
    localtime_s(&tm_now, &t_now);  // Windows
#else
    localtime_r(&t_now, &tm_now);  // Linux / Unix
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Service::ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander)
{
    databasethreadpool_.EnqueueTask(
        [this, query, rowhander](MysqlConnection& conn, DBCallback done) {
            LOG_DEBUG << query << " executing DB task";
            
            if (!conn.IsConnected()) {
                LOG_ERROR << "Database connection is not established for query: " << query;
                done(false);
                return;
            }

            try {
                MYSQL_RES* res = conn.ExcuteQuery(query);
                if (!res) {
                    LOG_ERROR << "Query failed: " << query;
                    done(false);
                    return;
                }
                
                MysqlResult result(res);
                int rowcount = 0;
                while (result.Next()) {
                    MysqlRow row = result.GetRow();
                    rowhander(row);
                    rowcount++;
                }
                
                LOG_DEBUG << "Query completed successfully: " << query
                    << ", processed " << rowcount << " rows";
                done(true);
                
            } catch (const std::exception& e) {
                LOG_ERROR << "Database operation failed for query: " << query << ", error: " << e.what();
                done(false);
            }
        },
        [query](bool success) {
            // 默认的空回调，保持原有的"fire and forget"行为
            if (success) {
                LOG_DEBUG << "ReadFromDataBase completed successfully for query: " << query;
            } else {
                LOG_ERROR << "ReadFromDataBase failed for query: " << query;
            }
        }
    );
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

Service::Service()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    InitIdsFromMySQL();
}

Service::~Service()
{
}

void Service::UserRegister(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    bool end = true;

    std::string username;
    std::string password;
    std::string email;

    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);
    end &= AssignIfPresent(js, "email", email);

    int userid = gen_.GetNextUserId();

    std::ostringstream oss;
    oss << "insert into users (id, email, name, password) values ("
        << userid << ", '" << email << "', '" << username << "', '"
        << password << "');";
    std::string query = oss.str();

    databasethreadpool_.EnqueueTask(
    [this, query](MysqlConnection& conn, DBCallback done) 
    {
        if (!conn.ExcuteUpdate(query))
            done(true);
        else 
            done(false);
    }, 
    [this, conn](bool succsee)mutable { 
            if (succsee) {
                LOG_DEBUG << "Register success";
            }
            else {
                LOG_ERROR << "Register failed";
            }

            json j = {
            {"end", succsee}
            };
            conn->send(code_.encode(j, "RegisterBack"));
        }
    );
}

void Service::UserLogin(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{
    std::string password;
    std::string email;
    AssignIfPresent(js, "password", password);
    AssignIfPresent(js, "email", email);

    // 输入验证
    if (email.empty() || password.empty()) {
        json j = {
            {"end", false},
            {"id", -1},
            {"name", ""},
            {"error", "Email and password are required"}
        };
        conn->send(code_.encode(j, "LoginBack"));
        return;
    }

    databasethreadpool_.EnqueueTask([this, email, password, conn](MysqlConnection& mysqlconn, DBCallback done)
    {
        std::string query = "SELECT id, name, password FROM users WHERE email = '" + email + "'";

        LOG_DEBUG << "Executing login query for email: " << email;

        try {
            MYSQL_RES* res = mysqlconn.ExcuteQuery(query);
            if (!res) {
                LOG_ERROR << "Login query failed for email: " << email;
                done(false);
                return;
            }

            MysqlResult result(res);
            bool login_success = false;
            int userid = -1;
            std::string username;

            if (result.Next()) 
            {
                MysqlRow row = result.GetRow();
                userid = row.GetInt("id");
                username = row.GetString("name");
                std::string db_password = row.GetString("password");
                
                if (db_password == password) 
                {
                    login_success = true;
                    LOG_INFO << "Login successful for user: " << username << " (ID: " << userid << ")";
                        
                    // 设置用户在线状态和连接
                    onlineuser_.AddUser(userid, username, password, email);
                    onlineuser_.SetOnline(userid);
                    onlineuser_.GetUser(userid)->SetConn(conn);

                    json success_response = {
                        {"end", true},
                        {"id", userid},
                        {"name", username}
                    };
                    conn->send(code_.encode(success_response, "LoginBack"));
                } 
                else 
                {
                    LOG_WARN << "Password mismatch for email: " << email;
                    login_success = false;
                }
            } 
            else 
            {
                LOG_WARN << "User not found for email: " << email;
                login_success = false;
            }

            if (!login_success) {
                json failure_response = {
                {"end", false},
                {"id", -1},
                {"name", ""},
                };
                conn->send(code_.encode(failure_response, "LoginBack"));
            }

            done(login_success);

        } catch (const std::exception& e) {
            LOG_ERROR << "Database error during login: " << e.what();
            done(false);          
            
            // 发送错误响应
            json error_response = {
                {"end", false},
                {"id", -1},
                {"name", ""},
                {"error", "Database error"}
            };
            conn->send(code_.encode(error_response, "LoginBack"));                
            done(false);
        }
    },
    [email](bool success) {
            if (success) {
                LOG_DEBUG << "Login database operation completed for: " << email;
            } else {
                LOG_ERROR << "Login database operation failed for: " << email;
            }
        }
    );
}

void Service::MessageSend(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{

    int senderid = 0;
    int receiverid = 0;
    std::string content;
    std::string type;
    std::string status;
    std::string timestamp = GetCurrentTimestamp();

    AssignIfPresent(js, "senderid", senderid);
    AssignIfPresent(js, "receiverid", receiverid);
    AssignIfPresent(js, "content", content);
    AssignIfPresent(js, "type", type);
    AssignIfPresent(js, "status", status);

    //添加到redis,获取自增id
    int msgid = gen_.GetNextMsgId();

    databasethreadpool_.EnqueueTask(
    [this, msgid, senderid, receiverid, content, type, status, timestamp](MysqlConnection& mysqlconn, DBCallback done)mutable 
    {
        try 
        {
            if (type == "Group")
            {

            }
            else if (type == "Private")
            {
                if (onlineuser_.IsOnline(receiverid))
                {
                    status = "Read";
                    auto receiver = onlineuser_.GetUser(receiverid);
                    if (receiver)
                    {
                        auto reconn = receiver->GetConn();
                        if (reconn)
                        {
                            json j = {
                            {"senderid", senderid},
                            {"receiverid", receiverid},
                            {"content", content},
                            {"type", type},
                            {"status", status},
                            {"timestamp", timestamp}
                            };
                            reconn->send(code_.encode(j, "RecvMessage"));
                        }
                    }
                }
                else  
                {

                }
            }
            else
            {
                LOG_ERROR << "Message type wrong";
            }

            std::ostringstream oss;
            oss << "insert into messages (id, senderid, receiverid, content, type, status, timestamp) values ("
                << msgid << ", " << senderid << ", " << receiverid << ", '" << content << "','"
                << type  << "','" << status << "','" << timestamp << "');";

            if (!mysqlconn.ExcuteUpdate(oss.str())) 
            {
                LOG_ERROR << "Failed to insert message into database, query: " << oss.str();
                done(false);
                return;  // 添加 return
            }
            
            std::cout << "---------" << std::endl;
            done(true);
        } catch(const std::exception& e)
        {
            LOG_ERROR << "Database error during message send: " << e.what();
            done(false);
        }

    }, [this, conn](bool success)mutable { 
        if (success) {
            LOG_DEBUG << "Message send database operation completed";
        }
        else { 
            LOG_ERROR << "Message send database operation failed";
        }
        json j = {
            {"end", success}
        };
        conn->send(code_.encode(j, "SendMessageBack"));
    });
}

void Service::SendFriendApply(const TcpConnectionPtr& conn, const json& js, Timestamp)
{
    bool end = true;

    int fromid;
    std::string targetemail;

    AssignIfPresent(js, "fromid", fromid);
    AssignIfPresent(js, "targetemail", targetemail);

    databasethreadpool_.EnqueueTask([this, fromid,targetemail](MysqlConnection& mysqlconn, DBCallback done) {
        std::string query = "select id from users where email = '" + targetemail + "';";
        MYSQL_RES* res = mysqlconn.ExcuteQuery(query);
        if (!res) 
        {
            LOG_ERROR << "Sendfriendapply query failed for email: " << targetemail;
            done(false);
            return;
        }
        
        MysqlResult result(res);
        MysqlRow row = result.GetRow();
        int targetid = row.GetInt("id");
        int applyid = gen_.GetNextFriendApplyId();

        std::ostringstream oss;
        oss << "insert into friendapplys (id, fromid, targetid, status) values ("
            << applyid << ", " << fromid << ", " << targetid << ",'" << "Pending"
            << "'); ";
        
        if (!mysqlconn.ExcuteUpdate(oss.str()))
            done(true);
        else 
            done(false);
        
    }, [this, conn](bool success) {
        if (success){
        }    LOG_DEBUG << "Sendapply success";
        else {
            LOG_ERROR << "Sendapply failed";
        }

        json j = {
            {"end", success}
        };
        conn->send(code_.encode(j, "SendApplyBack"));
    });
}

void Service::ListFriendAllApplys(const TcpConnectionPtr& conn, const json& js, Timestamp)
{
    int targetid;
    AssignIfPresent(js, "targetid", targetid);

    databasethreadpool_.EnqueueTask([this, conn, targetid](MysqlConnection& mysqlconn, DBCallback done) {
        std::ostringstream oss;
        oss << "select fromid , status from friendapplys where targetid = " << targetid;
        MYSQL_RES* res = mysqlconn.ExcuteQuery(oss.str());
        if (!res) 
        {
            LOG_ERROR << "Listfriendapply query failed ";
            done(false);
            return;
        }

        MysqlResult result(res);
        json j = json::array();
        int fromid;
        std::string fromname;
        std::string status;

        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            fromid = row.GetInt("fromid");
            status = row.GetString("status");

            std::string qu = "select name from users where id = " + std::to_string(fromid);
            MYSQL_RES* re = mysqlconn.ExcuteQuery(qu);
            if (!res) {
                LOG_ERROR << "Listfriendapply from name query failed ";
                done(false);
                return;
            }
            MysqlResult resul(re);
            fromname = resul.GetRow().GetString("name");

            json apply = {
                {"fromid", fromid},
                {"fromname", fromname},
                {"status", status}
            };
            j.push_back(apply);
        }
        j["end"] = true;
        conn->send(code_.encode(j, "AllApplyBack"));

    }, [this, conn](bool success) {
        if (success) {
            LOG_DEBUG << "Listfriendapply success";
        }
        else {
            LOG_ERROR << "Listfriendapply failed";
            json j = json::array();
            j["end"] = false;
            conn->send(code_.encode(j, "AllApplyBack"));
        }
    });
}

void Service::ListFriendSendApplys(const TcpConnectionPtr& conn, const json& js, Timestamp)
{
    int fromid;
    AssignIfPresent(js, "fromid", fromid);

    databasethreadpool_.EnqueueTask([this, conn, fromid](MysqlConnection& mysqlconn, DBCallback done) {
        std::ostringstream oss;
        oss << "select targetid, status from friendapplys where fromid = " << fromid;
        MYSQL_RES* res = mysqlconn.ExcuteQuery(oss.str());
        if (!res) {
            LOG_ERROR << "Listsendfriendapply query failed ";
            done(false);
            return;
        }

        MysqlResult result(res);
        json j = json::array();
        int targetid;
        std::string targetname;
        std::string status;

        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            targetid = row.GetInt("targetid");
            status = row.GetString("status");

            std::string qu = "select name from users where id = " + std::to_string(targetid);
            MYSQL_RES* re = mysqlconn.ExcuteQuery(qu);
            if (!res) {
                LOG_ERROR << "Listsendfriendapply fromname query failed ";
                done(false);
                return;
            }
            MysqlResult resul(re);
            targetname = resul.GetRow().GetString("name");

            json apply = {
                {"fromid", targetid},
                {"fromname", targetname},
                {"status", status}
            };
            j.push_back(apply);
        }
        j["end"] = true;
        conn->send(code_.encode(j, "AllSendApplyBack"));

    }, [this, conn](bool success) {
        if (success) {
            LOG_DEBUG << "Listsendfriendapply success";
        }
        else {
            LOG_ERROR << "Listsendfriendapply failed";
            json j = json::array();
            j["end"] = false;
            conn->send(code_.encode(j, "AllSendApplyBack"));
        }
    });
}

void Service::ProceFriendApplys(const TcpConnectionPtr& conn, const json& js, Timestamp)
{
    bool end = true;
    int fromid;
    int targetid;
    std::string status;

    AssignIfPresent(js, "fromid", fromid);
    AssignIfPresent(js, "targetid", targetid);
    AssignIfPresent(js, "status", status);

    databasethreadpool_.EnqueueTask([this, conn, targetid, fromid, status](MysqlConnection& mysqlconn, DBCallback done) {
        std::ostringstream oss;
        oss << "update friendapplys set status = '" << status << "'"
            << "where targetid = " << targetid << "and fromid = "
            << fromid << ";";
        if (mysqlconn.ExcuteUpdate(oss.str()))
            done(false);

        if (status == "Agree")
        {
            int fd = gen_.GetNextFriendId();
            std::ostringstream os;
            oss << "insert into friends (id, userid, friendid, block) values ("
                << fd << ", " << fromid << ", " << targetid << ", " << false << "); ";
            if (mysqlconn.ExcuteUpdate(os.str()))
                done(false);

            fd = gen_.GetNextFriendId();
            std::ostringstream o;
            oss << "insert into friends (id, userid, friendid, block) values ("
                << fd << ", " << targetid << ", " << fromid << ", " << false << "); ";
            if (mysqlconn.ExcuteUpdate(o.str()))
                done(false);
        }
        else if (status == "Reject")
        {
        }
        else
        {
            done(false);
        }

    }, [this, conn](bool success) {
        if (success) {
            LOG_DEBUG << "Processfriendapply success";
        }
        else {
            LOG_ERROR << "Processfriendapply failed";
        }
        json j = {
            {"end", success}
        };
        conn->send(code_.encode(j, "ProceApplyBack"));
    });
}

void Service::ListFriends(const TcpConnectionPtr& conn, const json& js, Timestamp)
{
    int userid;
    AssignIfPresent(js, "userid", userid);

    databasethreadpool_.EnqueueTask([this, conn, userid](MysqlConnection& mysqlconn, DBCallback done) {
        std::string query = "select friendid, block from friends where userid = " + std::to_string(userid);
        MYSQL_RES* res = mysqlconn.ExcuteQuery(query);
        if (!res) 
        {
            LOG_ERROR << "listfriend query failed for userid: " << userid;
            done(false);
            return;
        }

        MysqlResult result(res);
        json j = json::array();
        int friendid;
        std::string friendname;
        bool block;

        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            friendid = row.GetInt("id");
            block = row.GetBool("block");

            std::string qu = "select name from users where id = " + std::to_string(friendid);
            MYSQL_RES* re = mysqlconn.ExcuteQuery(qu);
            if (!res) {
                LOG_ERROR << "Listfriend from name query failed ";
                done(false);
                return;
            }
            MysqlResult resul(re);
            friendname = resul.GetRow().GetString("name");

            json fri = {
                {"friendid", friendid},
                {"friendname", friendname},
                {"block", block}
            };
            j.push_back(fri);
        }
        j["end"] = true;
        conn->send(code_.encode(j, "ListFriendBack"));

    }, [this, conn](bool success) {
        if (success){
        }    LOG_DEBUG << "Listfriends success";
        else {
            LOG_ERROR << "Listfriends failed";
            json j;
            j["end"] = false;
            conn->send(code_.encode(j, "ListFriendBack"));
        }
    });
}

void Service::GetChatHistory(const TcpConnectionPtr& conn, const json& js, Timestamp time)
{

}

