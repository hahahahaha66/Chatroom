#include "Friend.h"

Friend::Friend()
{
    status_ = Status::Normal;
}

Friend::Friend(int userid, int friendid, std::string status)
    : userid_(userid), friendid_(friendid)
{
   SetStatus(status);
}

Friend::Friend(int userid,  std::string friendname, int friendid, std::string status)
    : userid_(userid), friendid_(friendid)
{
    SetFriendName(friendname);
    SetStatus(status);
}


void Friend::SetUserId(int userid)
{
    userid_ = userid;
}

int Friend::GetUserId() const
{
    return userid_;
}

void Friend::SetFriendName(const std::string friendname)
{
    friendname_ = friendname;
}

std::string Friend::GetFriendName() const
{
    return friendname_;
}

void Friend::SetFriendId(int friendid)
{
    friendid_ = friendid;
}

int Friend::GetFriendId() const
{
    return friendid_;
}

void Friend::SetStatus(std::string status)
{
    if (status == "Block")
    {
        status_ = Status::Block;
    }
    else  
    {
        status_ = Status::Normal;
    }
}

std::string Friend::GetStatus() const
{
    if (status_ == Status::Block)
    {
        return "Block";
    }
    else  
    {
        return "Normal";
    }
}