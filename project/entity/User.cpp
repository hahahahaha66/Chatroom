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