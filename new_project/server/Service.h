#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/IdGenerator.h"
#include "../tool/Codec.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include "../model/User.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

class Service {
public:
    Service();
    ~Service();

    // Database
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander);
    void InitIdsFromMySQL();

    // User
    void UserRegister(const TcpConnectionPtr& conn, const json& js);
    void UserLogin(const TcpConnectionPtr& conn, const json& js);
    void RemoveUserConnect(const TcpConnectionPtr& conn);

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

    // other
    template<typename T>
    std::optional<T> ExtractCommonField(const json& j, const std::string& key);
    template<typename T>
    bool AssignIfPresent(const json& j, const std::string& key, T& out);
    std::string Escape(const std::string& input);
    std::string GetCurrentTimestamp();
    
private:
    std::atomic<bool> running_ = false;

    IdGenerator gen_;
    Codec code_;
    DatabaseThreadPool databasethreadpool_;

    std::unordered_map<int, User> onlineuser_;

};