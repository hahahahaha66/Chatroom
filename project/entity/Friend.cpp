#include "Friend.h"

Friend::Friend(int friendid, std::string friendname, bool online)
    : friendid_(friendid),
      friendname_(friendname),
      online_(online)
{
}

void Friend::SetFriendId(int& friendid)
{
    friendid_ = friendid;
}

const int Friend::GetFriendId() const 
{
    return friendid_;
}

void Friend::SetFriendName(std::string& friendname) 
{
    friendname_ = friendname;
}

const std::string& Friend::GetFriendName() const
{
    return friendname_;
}

void Friend::SetOnline(bool online)
{
    online_ = online;
}

const bool Friend::GetOnline() const
{
    return online_;
}