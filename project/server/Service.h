#ifndef SERVICE_H
#define SERVICE_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../tool/Dispatcher.h"
#include "../tool/Codec.h"
#include "../entity/User.h"
#include "../entity/Group.h"

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

class Service
{
public:
    Service(Dispatcher& dispatcher);
    ~Service(){};

    void RegisterAllHanders(Dispatcher& dispatcher);

    void ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void RegisterAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListFriendlist(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void DeleteFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void BlockFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void AddFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListFriendApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void CreateGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupMemberList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ListGroupApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);
    void ProcessFriendApply(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);

private:
    Codec codec_;
    Dispatcher& dispatcher_;

    std::unordered_map<int, User> userlist_;
    std::unordered_map<int, Group> grouplist_;
    std::unordered_map<uint16_t, MessageHander> handermap_;
};

#endif