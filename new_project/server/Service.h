#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/UserManager.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"

#include <memory>
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
    void UserRegister(const TcpConnectionPtr& conn, const json& json);
    void UserLogin(const TcpConnectionPtr& conn, const json& json);

    //Message
    void MessageSend(const TcpConnectionPtr& conn, const json& json);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& json);
    void GetGroupHistory(const TcpConnectionPtr& conn, const json& json);

    //Friend
    void SendFriendApply(const TcpConnectionPtr& conn, const json& json);
    void ListFriendApply(const TcpConnectionPtr& conn, const json& json);
    void ListSendFriendApply(const TcpConnectionPtr& conn, const json& json);
    void ProceFriendApply(const TcpConnectionPtr& conn, const json& json);
    void ListFriend(const TcpConnectionPtr& conn, const json& json);

    //Group
    void CreateGroup(const TcpConnectionPtr& conn, const json& json);
    void SendGroupApply(const TcpConnectionPtr& conn, const json& json);
    void ListGroupApply(const TcpConnectionPtr& conn, const json& json);
    void ProceGroupApply(const TcpConnectionPtr& conn, const json& json);
    void ListSendGroupApply(const TcpConnectionPtr& conn, const json& json);
    void ListGroupMember(const TcpConnectionPtr& conn, const json& json);
    void ListGroup(const TcpConnectionPtr& conn, const json& json);
    void QuitGroup(const TcpConnectionPtr& conn, const json& json);
    void ChangeUserRole(const TcpConnectionPtr& conn, const json& json);
    
private:
    std::atomic<bool> running_ = false;

    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;

    UserManager onlineuser_;

};