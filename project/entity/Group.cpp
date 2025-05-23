#include "Group.h"
#include "GroupUser.h"
#include <unordered_map>

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

void Group::AddApply(int applicantid)
{
    applylist_.insert(applicantid);
}

void Group::ApprovalApply(int applicantid)
{
    applylist_.erase(applicantid);
    members_[applicantid] = std::move(GroupUser(applicantid, "Member"));
}

void Group::RefuseApply(int applicantid)
{
    applylist_.erase(applicantid);
}

const std::unordered_set<int>& Group::GetApplyList() const
{
    return applylist_;
}

void Group::AddMember(const GroupUser& user)
{
    members_[user.GetUserId()] = user;
}

void Group::RemoveMember(int userid)
{
    members_.erase(userid);
}

bool Group::HasMember(int userid) const
{
    if (members_.find(userid) == members_.end())
        return false;
    else  
        return true;
}

const GroupUser* Group::GetMember(int userid) const
{
    return &(members_.at(userid));
}

const std::unordered_map<int, GroupUser>& Group::GetAllMembers() const
{
    return members_;
}