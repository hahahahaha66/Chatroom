#include "GroupUser.h"

GroupUser::GroupUser(int userid, const std::string& username, const std::string& role)
    : userid_(userid),
      username_(username),
      role_(role)
{
}

int GroupUser::GetUserId() const
{
    return userid_;
}

const std::string& GroupUser::GetUserName() const 
{
    return username_;
}

const std::string& GroupUser::GetRole() const
{
    return role_;
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
    role_ = role;
}

void GroupUser::SetMuted(bool muted)
{
    muted_ = muted;
}

void GroupUser::SetLastReadTime(time_t lastreadtime)
{
    lastreadtime_ = lastreadtime;
}