#ifndef GROUP_H
#define GROUP_H

#include "GroupUser.h"
#include "other.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Group  
{
public:
    Group() = default;
    Group(int groupid, const std::string& groupname);

    int GetGroupId() const;
    const std::string& GetGroupName() const;

    bool AddApply(int applicantid, std::string applyname);
    bool AddApplyId(int applicantid);
    bool ApprovalApply(int applicantid);
    bool DeleteApply(int applicantid);
    const std::unordered_map<int, SimpUser>& GetApplyList() const;
    
    bool AddMember(const GroupUser& user);
    bool RemoveMember(int userid);
    bool HasMember(int uerid) const;
    GroupUser* GetMember(int userid);
    const std::unordered_map<int, GroupUser>& GetAllMembers() const;

private:
    int groupid_;
    std::string groupname_;

    std::unordered_map<int, SimpUser> applylist_;
    std::unordered_map<int, GroupUser> members_;
    
};

#endif