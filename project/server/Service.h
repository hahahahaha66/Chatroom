#ifndef SERVICE_H
#define SERVICE_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../tool/Dispatcher.h"
#include "../tool/Codec.h"
#include "../entity/User.h"
#include "../entity/Group.h"
#include "../entity/Friend.h"
#include "../database/MysqlConnectionpool.h"
#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlResult.h"
#include "../database/MysqlRow.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, const uint16_t, Timestamp)>;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

struct ChatConnect 
{
    enum class Type {None, Private, Group};
    Type type_;
    int peerid_;
    ChatConnect(int peerid, Type type = Type::None) : type_(type), peerid_(peerid) {}
};

class Service
{
public:
    Service(Dispatcher& dispatcher);
    ~Service(){};

    void RegisterAllHanders(Dispatcher& dispatcher);
    
    void ReadUserFromDataBase();
    void ReadGroupFromDataBase();
    void ReadFriendFromDataBase();
    void ReadChatConnectFromDataBase();
    void ReadGroupApplyFromDataBase();
    void ReadUserApplyFromDataBase();
    void ReadGroupUserFromDataBase();

    void StartAutoFlushToDataBase(int seconds = 5);
    void StopAutoFlush();
    void FlushToDataBase();

    std::string Escape(const std::string& input);
    std::string FormatUpdateUser(const User& user);
    std::string FormatUpdateGroup(const Group& group);
    std::string FormatUpdateGroupUser(const int& groupid, const GroupUser& groupuser);
    std::string FormatUpdateFriend(const Friend& userfriend);
    std::string FormatUpdateUserApply(const int& userid, const int& applyid);
    std::string FormatUqdateGroupApply(const int& groupid, const int& applyid);

    void ProcessingLogin    (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void RegisterAccount    (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListFriendlist     (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void DeleteFriend       (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void BlockFriend        (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void AddFriend          (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListFriendApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupList      (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void CreateGroup        (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupMemberList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupApplyList (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ProcessFriendApply (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ProcessGroupApply  (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void QuitGroup          (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void PrintUserData      (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ChangeUserPassword (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void DeleteUserAccount  (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void DeleteGroup        (const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);

private:
    void ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowprocessor);

    std::atomic<bool> running_ = false;
    std::thread flush_thread_;
    Codec codec_;
    Dispatcher& dispatcher_;
    DatabaseThreadPool databasethreadpool_;

    std::unordered_map<int, ChatConnect> chatconnect_;
    std::unordered_map<int, std::unordered_map<int, Friend>> userfriendlist;
    std::unordered_map<int, User> userlist_;
    std::unordered_map<int, Group> grouplist_;
    std::unordered_map<uint16_t, MessageHander> handermap_;
};

#endif