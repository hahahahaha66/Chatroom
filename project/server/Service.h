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
#include "../entity/other.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, const int, Timestamp)>;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Service
{
public:
    Service(Dispatcher& dispatcher);
    ~Service();

    void RegisterAllHanders(Dispatcher& dispatcher);
    
    //从数据库中读取数据
    void ReadUserFromDataBase();
    void ReadGroupFromDataBase();
    void ReadFriendFromDataBase();
    void ReadChatConnectFromDataBase();
    void ReadGroupApplyFromDataBase();
    void ReadUserApplyFromDataBase();
    void ReadGroupUserFromDataBase();
    void ReadMessageFromDataBase();

    //启动另一线程在背后实现定期刷新
    void StartAutoFlushToDataBase(int seconds = 10);
    void StopAutoFlush();
    void FlushToDataBase();

    //格式化拼接刷新语句
    std::string Escape(const std::string& input);
    std::string FormatUpdateUser(const User& user);
    std::string FormatUpdateGroup(const Group& group);
    std::string FormatUpdateGroupUser(const int& groupid, const GroupUser& groupuser);
    std::string FormatUpdateFriend(const Friend& userfriend);
    std::string FormatUpdateUserApply(const int& userid, const int& applyid);
    std::string FormatUqdateGroupApply(const int& groupid, const int& applyid);
    std::string FormatUpdateMessage(const Message& message);

    //消息
    void ProcessMessage(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    //历史消息
    void GetUserChatInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void GetGroupChatInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);

    //登陆与注册
    void ProcessingLogin    (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void RegisterAccount    (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    
    //朋友操作
    void DeleteFriend       (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void BlockFriend        (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void AddFriend          (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    
    //群聊操作
    void AddGroup           (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void CreateGroup        (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void QuitGroup          (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void RemoveGroupUser    (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void SetAdministrator   (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void RemoveAdministrator(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);

    //处理申请
    void ProcessFriendApply (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void ProcessGroupApply  (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);

    //删除
    void DeleteUserAccount  (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);
    void DeleteGroup        (const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);

    //更新用户当前界面
    void UpdatedUserInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time);

    //刷新机制
    void RefreshFriendDelete(int userid, int friendid, Timestamp time);
    void RefreshFriendBlock(int userid, int friendid, Timestamp time);
    void RefreshFriendAddApply(int userid, int friendid, Timestamp time);
    void RefreshGroupAddApply(int groupid, int userid, Timestamp time);
    void RefreshGroupCreate(int groupid, Timestamp time);
    void RefreshFriendApplyProcess(int userid, int friendid, bool result, Timestamp time);
    void RefreshGroupApplyProcess(int groupid, int userid, bool result, Timestamp time);
    void RefreshGroupQuitUser(int groupid, int userid, Timestamp time);
    void RefreshGroupRemoveUser(int groupid, int userid, Timestamp time);
    void RefreshGroupAddAdministrator(int groupid, int userid, Timestamp time);
    void RefreshGroupRemoveAdministrator(int groupid, int userid, Timestamp time);

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
    std::unordered_map<int, std::unordered_map<int, std::vector<Message>>> usermessage_;
    std::unordered_map<int, std::vector<Message>> groupmessage_;
    std::unordered_map<int, MessageHander> handermap_;
};

#endif