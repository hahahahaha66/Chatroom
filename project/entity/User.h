#ifndef USER_H
#define USER_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "GroupUser.h"
#include <memory>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

class User 
{
public:
    User(int userid, const std::string& username);

    int GetId() const;
    const std::string& GetUserName() const;

    void SetOnline(std::shared_ptr<TcpConnection> conn) ;
    void SetOffline();
    bool IsOnLine() const;
    std::shared_ptr<TcpConnection> GetConnection() const;

    void JoinGroup(int groupid);
    void LeaveGroup(int groupid);
    bool IsInGroup(int groupid) const;
    const std::unordered_set<int>& GetGroupIds() const;

private:
    int userid_;
    std::string username_;
    bool online_ = false;
    std::shared_ptr<TcpConnection> conn_;

    std::unordered_set<int> groupids_;
};

#endif