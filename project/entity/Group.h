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

    void AddApply(int applicantid);
    void ApprovalApply(int applicantid);
    void RefuseApply(int applicantid);
    const std::unordered_set<int>& GetApplyList() const;
    
    void AddMember(const GroupUser& user);
    void RemoveMember(int userid);
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