#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/net/EventLoop.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../tool/Json.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include "../model/User.h"

#include <hiredis/hiredis.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis.h>
#include <unordered_map>

using json = nlohmann::json;

class Service {
public:
    Service(EventLoop* loop);
    ~Service();

    // Database
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander);
    void InitIdsFromMySQL();
    void FlushMessageToMySQL();

    // User
    void UserRegister(const TcpConnectionPtr& conn, const json& js);
    void UserLogin(const TcpConnectionPtr& conn, const json& js);
    void DeleteAccount(const TcpConnectionPtr& conn, const json& js);
    void RemoveUserConnect(const TcpConnectionPtr& conn);
    std::unordered_map<int, User> GetOnlineUserList() { return onlineuser_; }
    void TimeoutDetection();

    // flush
    void Flush(const TcpConnectionPtr& conn, const json& js);

    // Message
    void MessageSend(const TcpConnectionPtr& conn, const json& js);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& js);
    void GetGroupHistory(const TcpConnectionPtr& conn, const json& js);
    void ChanceInterFace(const TcpConnectionPtr& conn, const json& js);

    // Friend
    void SendFriendApply(const TcpConnectionPtr& conn, const json& js);
    void ListFriendApply(const TcpConnectionPtr& conn, const json& js);
    void ListSendFriendApply(const TcpConnectionPtr& conn, const json& js);
    void ProceFriendApply(const TcpConnectionPtr& conn, const json& js);
    void ListFriend(const TcpConnectionPtr& conn, const json& js);
    void BlockFriend(const TcpConnectionPtr& conn, const json& js);
    void DeleteFriend(const TcpConnectionPtr& conn, const json& js);

    // Group
    void CreateGroup(const TcpConnectionPtr& conn, const json& js);
    void SendGroupApply(const TcpConnectionPtr& conn, const json& js);
    void ListGroupApply(const TcpConnectionPtr& conn, const json& js);
    void ProceGroupApply(const TcpConnectionPtr& conn, const json& js);
    void ListSendGroupApply(const TcpConnectionPtr& conn, const json& js);
    void ListGroupMember(const TcpConnectionPtr& conn, const json& js);
    void ListGroup(const TcpConnectionPtr& conn, const json& js);
    void QuitGroup(const TcpConnectionPtr& conn, const json& js);
    void ChangeUserRole(const TcpConnectionPtr& conn, const json& js);
    void DeleteGroup(const TcpConnectionPtr& conn, const json& js);
    void BlockGroupUser(const TcpConnectionPtr& conn, const json& js);
    void RemoveGroupUser(const TcpConnectionPtr& conn, const json& js);

    // File
    void AddFileToMessage(const TcpConnectionPtr& conn, const json& js);
    void ListFriendFile(const TcpConnectionPtr& conn, const json& js);
    void ListGroupFile(const TcpConnectionPtr& conn, const json& js);

    std::string Escape(const std::string& input);
    std::string GetCurrentTimestamp();
    
private:
    EventLoop* loop_;

    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;
    sw::redis::Redis redis_;

    std::unordered_map<int, User> onlineuser_;
};