#include "Group.h"
#include "GroupUser.h"
#include "other.h"
#include <unordered_map>
#include <utility>

Group::Group(int groupid, const std::string& groupname)
    : groupid_(groupid),
      groupname_(groupname)
{
}

int Group::GetGroupId() const 
{
    return groupid_;
}

const std::string& Group::GetGroupName() const
{
    return groupname_;
}

bool Group::AddApply(int applicantid, std::string applyname)
{
    if (applylist_.insert(std::make_pair(applicantid, std::move(SimpUser(applicantid, applyname)))).second == true)
        return true;
    else  
        return false;
}

bool Group::AddApplyId(int applicantid)
{
    if (applylist_.insert(std::make_pair(applicantid, std::move(SimpUser(applicantid)))).second == true)
        return true;
    else  
        return false;
}

bool Group::ApprovalApply(int applicantid)
{
    if (applylist_.erase(applicantid) == 1)
    {
        members_[applicantid] = std::move(GroupUser(applicantid, "Member"));
        return true;
    }
    else  
    {
        return false;
    }
}

bool Group::DeleteApply(int applicantid)
{
    if (applylist_.erase(applicantid) == 1)
        return true;
    else  
        return false;
}

const std::unordered_map<int, SimpUser>& Group::GetApplyList() const
{
    return applylist_;
}

bool Group::AddMember(const GroupUser& user)
{
    if (members_.insert(std::make_pair(user.GetUserId(), user)).second == true)
        return true;
    else  
        return false;
}

bool Group::RemoveMember(int userid)
{
    if (members_.erase(userid) == 1)
        return true;
    else  
        return false;
}

bool Group::HasMember(int userid) const
{
    if (members_.find(userid) == members_.end())
        return false;
    else  
        return true;
}

GroupUser* Group::GetMember(int userid)
{
    return &(members_.at(userid));
}

const std::unordered_map<int, GroupUser>& Group::GetAllMembers() const
{
    return members_;
}