#ifndef USER_H
#define USER_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "GroupUser.h"
#include "Friend.h"
#include "other.h"

#include <memory>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class User 
{
public:
    User() = default;

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

    bool AddApply(int applicantid, std::string applyname);
    bool AddApplyId(int applicantid);
    void ApprovalApply(int userid, int applicantid);
    void DeleteApply(int applicantid);
    bool IsApply(int applicantid);
    const std::unordered_map<int, SimpUser>& GetApplyList() const;

    bool JoinGroup(int groupid);
    bool LeaveGroup(int groupid);
    bool IsInGroup(int groupid) const;
    const std::unordered_set<int>& GetGroupList() const;

private:
    int userid_;
    std::string username_;
    std::string password_;
    bool online_ = false;
    std::shared_ptr<TcpConnection> conn_;

    std::unordered_map<int, SimpUser> applylist_;
    std::unordered_set<int> grouplist_;
};

#endif