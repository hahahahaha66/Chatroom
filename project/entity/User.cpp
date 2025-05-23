#include "User.h"
#include <unordered_set>

User::User(int userid, const std::string& username, const std::string& passsword) 
    : userid_(userid),
      username_(username),
      password_(passsword)
{
}

int User::GetId() const
{
    return userid_;
}

void User::SetUserName(std::string& username)
{
    username_ = username;
}

const std::string& User::GetUserName() const
{
    return username_;
}

void User::SetPassWord(std::string& password)
{
    password_ = password;
}

const std::string& User::GetPassWord() const
{
    return password_;
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

void User::AddFriend(int friendid)
{
    friendlist_.insert(friendid);
}

void User::DeleteFriend(int friendid)
{
    friendlist_.erase(friendid);
}

bool User::IsFriend(int friendid) const
{
    if (friendlist_.find(friendid) == friendlist_.end())
        return false;
    else  
        return true;
}

const std::unordered_set<int>& User::GetFriendList() const
{
    return friendlist_;
}

void User::AddBlockFriend(int blockuser)
{
    blocklist_.insert(blockuser);
}

void User::DeleteBlockFriend(int blockuser)
{
    blocklist_.erase(blockuser);
}

bool User::IsBlockFriend(int blockuser) const
{
    if (blocklist_.find(blockuser) == blocklist_.end())
        return false;
    else  
        return true;
}

const std::unordered_set<int>& User::GetBlockList() const
{
    return blocklist_;
}

void User::AddApply(int applicantid)
{
    applylist_.insert(applicantid);
}

void User::ApprovalApply(int applicantid)
{
    applylist_.erase(applicantid);
    friendlist_.insert(applicantid);
}

void User::RefuseApply(int applicantid)
{
    applylist_.erase(applicantid);
}

const std::unordered_set<int>& User::GetApplyList() const
{
    return applylist_;
}

void User::JoinGroup(int groupid)
{
    grouplist_.insert(groupid);
}

void User::LeaveGroup(int groupid)
{
    grouplist_.erase(groupid);
}

bool User::IsInGroup(int groupid) const
{
    if (grouplist_.find(groupid) == grouplist_.end())
        return false;
    else  
        return true;
}

const std::unordered_set<int>& User::GetGroupList() const
{
    return grouplist_;
}