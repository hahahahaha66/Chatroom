#include "User.h"
#include "Friend.h"
#include "other.h"
#include <type_traits>
#include <unordered_set>
#include <utility>

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
    friendlist_[friendid] = std::move(Friend(userid_, friendid));
    if (friendlist_.insert(std::make_pair(friendid, std::move(Friend(userid_, friendid)))).second == true)
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

const std::unordered_map<int, Friend>& User::GetFriendList() const
{
    return friendlist_;
}

void User::SetFriendname(int friendid, std::string friendname)
{
    friendlist_[friendid].SetFriendName(friendname);
}

void User::SetStatusFriend(int friendid, std::string& status)
{
    friendlist_[friendid].SetStatus(status);
}

bool User::IsBlockFriend(int blockuserid) const
{
    if (friendlist_.at(blockuserid).GetStatus() == "Block")
        return true;
    else  
        return false;
}

const std::vector<int> User::GetBlockList() const
{
    std::vector<int> result;
    for (auto& it : friendlist_)
    {
        if (it.second.GetStatus() == "Block")
        {
            result.push_back(it.second.GetFriendId());
        }
    }
    return result;
}

bool User::AddApply(int applicantid, std::string applyname)
{
    if (applylist_.insert(std::make_pair(applicantid, std::move(SimpUser(applicantid, applyname)))).second == true)
        return true;
    else  
        return false;
}

bool User::AddApplyId(int applicantid)
{
    if (applylist_.insert(std::make_pair(applicantid, std::move(SimpUser(applicantid)))).second == true)
        return true;
    else  
        return false;
}

void User::ApprovalApply(int userid, int applicantid)
{
    applylist_.erase(applicantid);
    friendlist_.insert(std::make_pair(userid, std::move(Friend(userid_, applicantid))));
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

const std::unordered_map<int, SimpUser>& User::GetApplyList() const
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