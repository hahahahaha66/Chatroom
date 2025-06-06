#ifndef GROUP_H
#define GROUP_H

#include "GroupUser.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Group  
{
public:
    Group(int groupid, const std::string& groupname);

    int GetGroupId() const;
    const std::string& GetGroupName() const;

    bool AddApply(int applicantid);
    bool ApprovalApply(int applicantid);
    bool DeleteApply(int applicantid);
    const std::unordered_set<int>& GetApplyList() const;
    
    bool AddMember(const GroupUser& user);
    bool RemoveMember(int userid);
    bool HasMember(int uerid) const;
    const GroupUser* GetMember(int userid) const;
    const std::unordered_map<int, GroupUser>& GetAllMembers() const;

private:
    int groupid_;
    std::string groupname_;

    std::unordered_set<int> applylist_;
    std::unordered_map<int, GroupUser> members_;
    
};

#endif