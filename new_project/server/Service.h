#pragma one

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/UserManager.h"
#include "../manager/MessageManager.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include "../manager/FriendManager.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service {
public:
    Service();
    ~Service();

    //Database
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander);
    void InitIdsFromMySQL();
    void ReadUserFromDB();
    void ReadMessageFromDB();
    void ReadFriendFromDB();

    //flush
    void StartAutoFlushToDataBase(int seconds);
    void FlushToDataBase();
    void StopAutoFlush();
    std::string FormatUpdateUser(std::shared_ptr<User> user);
    std::string FormatUpdateMessage(std::shared_ptr<Message> message);
    std::string FormatUpdataFriend(std::shared_ptr<Friend> frienda);

    //User
    void UserRegister(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void UserLogin(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Message
    void MessageSend(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Friend
    //Group
    
private:
    std::atomic<bool> running_ = false;
    std::thread flush_thread_;

    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;

    UserManager usermanager_;
    MessageManager messagemanager_;
    FriendManager friendmanager_;
};