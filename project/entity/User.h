#ifndef USER_H
#define USER_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "GroupUser.h"

#include <memory>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class User 
{
public:
    User(int userid, const std::string& username, const std::string& password);

    int GetId() const;
    
    void SetUserName(std::string& username);
    const std::string& GetUserName() const;

    void SetPassWord(std::string& password);
    const std::string& GetPassWord() const;

    void SetOnline(std::shared_ptr<TcpConnection> conn) ;
    void SetOffline();
    bool IsOnLine() const;
    std::shared_ptr<TcpConnection> GetConnection() const;

    void AddFriend(int friendid);
    void DeleteFriend(int friendid);
    bool IsFriend(int friendid) const;
    const std::unordered_set<int>& GetFriendList() const;

    void AddBlockFriend(int blockuser);
    void DeleteBlockFriend(int blockuser);
    bool IsBlockFriend(int blockuser) const;
    const std::unordered_set<int>& GetBlockList() const;

    void AddApply(int applicantid);
    void ApprovalApply(int applicantid);
    void DeleteApply(int applicantid);
    const std::unordered_set<int>& GetApplyList() const;

    void JoinGroup(int groupid);
    void LeaveGroup(int groupid);
    bool IsInGroup(int groupid) const;
    const std::unordered_set<int>& GetGroupList() const;

private:
    int userid_;
    std::string username_;
    std::string password_;
    bool online_ = false;
    std::shared_ptr<TcpConnection> conn_;

    std::unordered_set<int> applylist_;
    std::unordered_set<int> blocklist_;
    std::unordered_set<int> friendlist_;
    std::unordered_set<int> grouplist_;
};

#endif