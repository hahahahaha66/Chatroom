#pragma one

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/UserManager.h"
#include "../manager/MessageManager.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service {
public:
    Service();

    //Database
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander);

    //User
    void UserRegister(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void UserLogin(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Message
    void MessageSend(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Friend
    //Group
    
private:
    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;

    UserManager usermanager_;
    MessageManager messagemanager_;
};