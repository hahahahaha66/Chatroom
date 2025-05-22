#include "User.h"

User::User(int userid, const std::string& username) 
    : userid_(userid),
      username_(username)
{
}

int User::GetId() const
{
    return userid_;
}

const std::string& User::GetUserName() const
{
    return username_;
}

void User::SetOnline(std::shared_ptr<TcpConnection> conn)
{
    conn_ = conn;
    online_ = true;
}

void User::SetOffline()
{
    online_ = false;
}

bool User::IsOnLine() const
{
    return online_;
}

std::shared_ptr<TcpConnection> User::GetConnection() const
{
    return conn_;
}

void User::JoinGroup(int groupid)
{
    groupids_.insert(groupid);
}

void User::LeaveGroup(int groupid)
{
    groupids_.erase(groupid);
}

bool User::IsInGroup(int groupid) const
{
    if (groupids_.find(groupid) == groupids_.end())
        return false;
    else  
        return true;
}

const std::unordered_set<int>& User::GetGroupIds() const
{
    return groupids_;
}