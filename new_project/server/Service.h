#include <memory>
#pragma one

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/UserManager.h"
#include "../manager/MessageManager.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include "../manager/FriendManager.h"
#include "../manager/FriendApplyManager.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service {
public:
    Service();
    ~Service();

    //Database
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander);
    void InitIdsFromMySQL();

    //User
    void UserRegister(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void UserLogin(const TcpConnectionPtr& conn, const json& json, Timestamp);

    //Message
    void MessageSend(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void GetGroupHistory(const TcpConnectionPtr& conn, const json& json, Timestamp);

    //Friend
    void SendFriendApply(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListFriendAllApplys(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListFriendSendApplys(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ProceFriendApplys(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListFriends(const TcpConnectionPtr& conn, const json& json, Timestamp);

    //Group
    void CreateGroup(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void SendGroupApply(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListGroupApply(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListGroupSendApply(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListGroupMember(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ListGroup(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void QuitGroup(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void ChangeUserRole(const TcpConnectionPtr& conn, const json& json, Timestamp);
    
private:
    std::atomic<bool> running_ = false;

    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;

    UserManager onlineuser_;

};