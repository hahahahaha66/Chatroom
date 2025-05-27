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

bool User::AddFriend(int friendid)
{
    if (friendlist_.insert(friendid).second == true)
        return false;
    else  
        return true;
}

bool User::DeleteFriend(int friendid)
{
    if (friendlist_.erase(friendid) == 1)
        return true;
    else  
        return false;
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

bool User::AddBlockFriend(int blockuser)
{
    if (blocklist_.insert(blockuser).second == true)
        return true;
    else
        return false;
}

bool User::DeleteBlockFriend(int blockuser)
{
    if (blocklist_.erase(blockuser) == 1)
        return true;
    else  
        return false;
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

bool User::AddApply(int applicantid)
{
    if (applylist_.insert(applicantid).second == true)
        return true;
    else  
        return false;
}

void User::ApprovalApply(int applicantid)
{
    applylist_.erase(applicantid);
    friendlist_.insert(applicantid);
}

void User::DeleteApply(int applicantid)
{
    applylist_.erase(applicantid);
}

bool User::IsApply(int applicantid)
{
    if (applylist_.find(applicantid) == applylist_.end())
        return false;
    else  
        return true;
}

const std::unordered_set<int>& User::GetApplyList() const
{
    return applylist_;
}

bool User::JoinGroup(int groupid)
{
    if (grouplist_.insert(groupid).second == true)
        return true;
    else  
        return false;
}

bool User::LeaveGroup(int groupid)
{
    if (grouplist_.erase(groupid) == 1)
        return true;
    else  
        return false;
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