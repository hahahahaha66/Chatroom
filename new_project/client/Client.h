#pragma once

#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include "../tool/Json.h"
#include "../test.cpp"
#include "../ftp/FileClient.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

namespace fs = std::filesystem;

struct Friend {
    Friend(int id, std::string name, std::string email, bool online, bool block)
        : id_(id), name_(name), block_(block), online_(online){}
    Friend() = default;
    int id_;
    std::string name_;
    std::string email_;
    bool online_;
    bool block_;
    bool new_ = false;
    std::string maxmsgtime_;
};

struct FriendApply {
    FriendApply(int id, std::string name, std::string status, std::string email)
        : id_(id), name_(name), status_(status), email_(email) {}
    FriendApply() = default;
    int id_;
    std::string name_;
    std::string email_;
    std::string status_;
};

// Member Administrator Owner
struct Group {
    Group(int id, std::string name, std::string role, bool mute)
        : id_(id), name_(name), role_(role), mute_(mute) {}
    Group() = default;
    int id_;
    std::string name_;
    std::string role_;
    bool mute_;
    bool newmessage_ = false;
    bool newapply_ = false;
    std::string maxmsgtime_;
};

struct File {
    File(int senderid, std::string filename, int64_t filesize, std::string timestamp)
        : senderid_(senderid), filename_(filename), filesize_(filesize), timestamp_(timestamp) {}
    File() = default;
    int senderid_;
    std::string filename_;
    int64_t filesize_;
    std::string timestamp_;
};

class Client 
{
public:
    Client(EventLoop& loop, uint16_t port, std::string ip, std::string name);
    
    void ConnectionCallBack(const TcpConnectionPtr& conn);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);

    void Register(const json& js);
    void Login(const json& js);
    void RegisterBack(const TcpConnectionPtr& conn, const json& js);
    void LoginBack(const TcpConnectionPtr& conn, const json& js);
    void DeleteAccount(const json& js);
    void DeleteAccountBack(const TcpConnectionPtr& conn, const json& js);

    // 刷新
    void StartFlushFromServer(int seconds);
    void Flush();
    void FlushBack(const TcpConnectionPtr& conn, const json& js);
    void StopFlushFromServer();

    // 消息
    void SendMessage(const json& js);
    void SendApply(const json& js);
    void SendMessageBack(const TcpConnectionPtr& conn, const json& js);
    void RecvMessageBack(const TcpConnectionPtr& conn, const json& js);
    void GetChatHistory(const json& js);
    void GetChatHistoryBack(const TcpConnectionPtr& conn, const json& js);
    void GetGroupHistory(const json& js);
    void GetGroupHistoryBack(const TcpConnectionPtr& conn, const json& js);
    void ChanceInterFace(const json& js);
    void ChanceInterFaceBack(const TcpConnectionPtr& conn, const json& js);

    // 好友
    void ListAllApply(const json& js);
    void ListAllSendApply(const json& js);
    void ProceFriendApply(const json& js);
    void ListFriend(const json& js);
    void BlockFriend(const json& js);
    void DeleteFriend(const json& js);

    void SendFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListSendFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ProceFriendApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListFriendBack(const TcpConnectionPtr& conn, const json& js);
    void BlockFriendBack(const TcpConnectionPtr& conn, const json& js);
    void DeleteFriendBack(const TcpConnectionPtr& conn, const json& js);

    // 群聊
    void CreateGroup(const json& js);
    void SendGroupApply(const json& js);
    void ListGroupApply(const json& js);
    void ProceGroupApply(const json& js);
    void ListSendGroupApply(const json& js);
    void GroupMember(const json& js);
    void ListGroup(const json& js);
    void QuitGroup(const json& js);
    void ChangeUserRole(const json& js);
    void DeleteGroup(const json& js);
    void BlockGroupUser(const json& js);

    void CreateGroupBack(const TcpConnectionPtr& conn, const json& js);
    void SendGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ProceGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void ListSendGroupApplyBack(const TcpConnectionPtr& conn, const json& js);
    void GroupMemberBack(const TcpConnectionPtr& conn, const json& js);
    void ListGroupBack(const TcpConnectionPtr& conn, const json& js);
    void QuitGroupBack(const TcpConnectionPtr& conn, const json& js);
    void ChangeUserRoleBack(const TcpConnectionPtr& conn, const json& js);
    void DeleteGroupBack(const TcpConnectionPtr& conn, const json& js);
    void BlockGroupUserBack(const TcpConnectionPtr& conn, const json& js);

    // 文件
    void GetFileServerPort(const json& js);
    void ListFriendFile(const json& js);
    void ListGroupFile(const json& js);
    void UploadFile(std::string filename, std::string filepath, int receiverid, std::string type);
    void DownloadFile(std::string filename, std::string savepath, std::string timestamp);
    
    void GetFileServerPortBack(const TcpConnectionPtr& conn, const json& js);
    void ListFriendFileBack(const TcpConnectionPtr& conn, const json& js);
    void ListGroupFileBack(const TcpConnectionPtr& conn, const json& js);

    std::string GetCurrentTimestamp();
    bool IsEarlier(const std::string& ts1, const std::string& ts2);
    bool ReadNum(std::string input, int &result);
    void PrintfRed(char a);
    void ClearScreen();

    void start();
    void stop();
    void send(const std::string& msg);
    inline void waitInPutReady();
    inline void notifyInputReady();

    void InputLoop();

private:
    std::atomic<bool> waitingback_ = false;
    std::string currentState_ = "init_menu";
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread flush_thread_;
    bool running_;

    TcpClient client_;
    Codec codec_;
    Dispatcher dispatcher_;
    Warning warning_;
    std::string ip_;
    uint16_t mainserverport_ = -1;
    uint16_t fileserverport_ = -1;
    EventLoop* loop_;

    std::shared_ptr<FileUploader> uploader_;
    std::shared_ptr<FileDownloader> downloader_;

    int userid_ = 0;
    std::string email_;
    std::string name_;
    std::string password_;

    std::unordered_map<int, Friend> friendlist_;
    std::unordered_multimap<int, FriendApply> friendapplylist_;
    std::unordered_multimap<int, FriendApply> friendsendapplylist_;
    std::unordered_map<int, Group> grouplist_;
    std::vector<File> filelist_;
};
