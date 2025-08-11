#pragma once

#include "../database/DatabaseThreadPool.h"
#include "../database/MysqlConnectionpool.h"
#include "../database/MysqlResult.h"
#include "../manager/IdGenerator.h"
#include "../model/User.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/TcpConnection.h"
#include "../tool/Codec.h"
#include "../tool/Json.h"
#include "../tool/ThreadPool.h"

#include <curl/curl.h>
#include <hiredis/hiredis.h>
#include <memory>
#include <mutex>
#include <mysql/mysql.h>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis.h>
#include <unordered_map>

using json = nlohmann::json;

struct SendContext {
    std::queue<std::string> pendingMessages;
    bool sending = false;  
};

struct Message {
    Message(int &senderid, std::string &content, std::string &status,
            std::string &timestamp)
        : senderid(senderid), content(content), status(status),
          timestamp(timestamp) {}
    int senderid;
    std::string content;
    std::string status;
    std::string timestamp;
};

class Service {
  public:
    Service(EventLoop *loop);
    ~Service();

    // Database
    void ReadFromDataBase(const std::string &query,
                          std::function<void(MysqlRow &)> rowhander);
    void InitIdsFromMySQL();
    void FlushMessageToRedis();
    void FlushMessageToMySQL();

    // User
    void UserRegister(const TcpConnectionPtr &conn, const json &js);
    void UserLogin(const TcpConnectionPtr &conn, const json &js);
    void DeleteAccount(const TcpConnectionPtr &conn, const json &js);
    void RemoveUserConnect(const TcpConnectionPtr &conn);
    std::unordered_map<int, User> &GetOnlineUserList() { return onlineuser_; }
    std::unordered_map<TcpConnectionPtr, SendContext> &GetMessageSendList() { return messagesendlist_; }
    void TimeoutDetection();

    // flush
    void Flush(const TcpConnectionPtr &conn, const json &js);

    // Message
    void MessageSend(const TcpConnectionPtr &conn, const json &js);
    void GetChatHistory(const TcpConnectionPtr &conn, const json &js);
    void GetGroupHistory(const TcpConnectionPtr &conn, const json &js);
    void ChanceInterFace(const TcpConnectionPtr &conn, const json &js);
    void SendVerifyCode(const TcpConnectionPtr &conn, const json &js);

    // Friend
    void SendFriendApply(const TcpConnectionPtr &conn, const json &js);
    void ListFriendApply(const TcpConnectionPtr &conn, const json &js);
    void ListSendFriendApply(const TcpConnectionPtr &conn, const json &js);
    void ProceFriendApply(const TcpConnectionPtr &conn, const json &js);
    void ListFriend(const TcpConnectionPtr &conn, const json &js);
    void BlockFriend(const TcpConnectionPtr &conn, const json &js);
    void DeleteFriend(const TcpConnectionPtr &conn, const json &js);
    void CheckFriendBlock(const TcpConnectionPtr &conn, const json &js);

    // Group
    void CreateGroup(const TcpConnectionPtr &conn, const json &js);
    void SendGroupApply(const TcpConnectionPtr &conn, const json &js);
    void ListGroupApply(const TcpConnectionPtr &conn, const json &js);
    void ProceGroupApply(const TcpConnectionPtr &conn, const json &js);
    void ListSendGroupApply(const TcpConnectionPtr &conn, const json &js);
    void ListGroupMember(const TcpConnectionPtr &conn, const json &js);
    void ListGroup(const TcpConnectionPtr &conn, const json &js);
    void QuitGroup(const TcpConnectionPtr &conn, const json &js);
    void ChangeUserRole(const TcpConnectionPtr &conn, const json &js);
    void DeleteGroup(const TcpConnectionPtr &conn, const json &js);
    void BlockGroupUser(const TcpConnectionPtr &conn, const json &js);
    void RemoveGroupUser(const TcpConnectionPtr &conn, const json &js);
    void AddFriendToGroup(const TcpConnectionPtr &conn, const json &js);

    // File
    void AddFileToMessage(const TcpConnectionPtr &conn, const json &js);
    void ListFriendFile(const TcpConnectionPtr &conn, const json &js);
    void ListGroupFile(const TcpConnectionPtr &conn, const json &js);

    // Person
    void ChancePassword(const TcpConnectionPtr &conn, const json &js);
    void ChanceUsername(const TcpConnectionPtr &conn, const json &js);

    std::string Escape(const std::string &input);
    std::string GetCurrentTimestamp();
    void CooldownTimeDecrement();
    int RandomDigitNumber();
    bool SendEmail(std::string &toemail, std::string &title, std::string &body);

  private:
    EventLoop *loop_;

    IdGenerator gen_;
    Codec code_;
    ThreadPool threadpool_;
    DatabaseThreadPool databasethreadpool_;
    sw::redis::Redis redis_;

    std::mutex mutex_;
    std::unordered_map<int, User> onlineuser_;
    std::vector<std::string> messagecachelist_;
    std::unordered_map<TcpConnectionPtr, SendContext> messagesendlist_;
    std::unordered_map<std::string, int> tempverificode_;
};