#include "GroupUser.h"

GroupUser::GroupUser(int userid, const std::string& role)
    : userid_(userid)
{
    SetRole(role);
}

int GroupUser::GetUserId() const
{
    return userid_;
}

const std::string GroupUser::GetRole() const
{
    switch (role_) {
        case Level::Group_owner:    return "Group_owner";
        case Level::Administrator:  return "Administrator";
        case Level::Member:         return "Member";
        default:                    return "Member";
    }
}

bool GroupUser::IsMuted() const
{
    return muted_;
}

time_t GroupUser::GetJoinTime() const
{
    return jointime_;
}

time_t GroupUser::GetLastReadTimre() const
{
    return lastreadtime_;
}

void GroupUser::SetRole(const std::string& role)
{
    if (role == "Group_owner") role_ = Level::Group_owner;
    else if (role == "Administrator") role_ = Level::Administrator;
    else role_ = Level::Member;
}

void GroupUser::SetMuted(bool muted)
{
    muted_ = muted;
}

void GroupUser::SetLastReadTime(time_t lastreadtime)
{
    lastreadtime_ = lastreadtime;
}