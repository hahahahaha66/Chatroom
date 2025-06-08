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
    std::unordered_set<int>& GetApplyList();
    
    bool AddMember(const GroupUser& user);
    bool RemoveMember(int userid);
    bool HasMember(int uerid) const;
    GroupUser* GetMember(int userid);
    std::unordered_map<int, GroupUser>& GetAllMembers();

private:
    int groupid_;
    std::string groupname_;

    std::unordered_set<int> applylist_;
    std::unordered_map<int, GroupUser> members_;
    
};

#endif