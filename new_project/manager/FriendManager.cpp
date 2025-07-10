#include "FriendManager.h"
#include <memory>

void FriendManager::AddFriend(int userid, int friendid)
{
    Friend fd(userid, friendid);
    friends_[userid][friendid] = std::make_shared<Friend>(fd);
}

void FriendManager::RemoveFriend(int userid, int friendid)
{
    friends_[userid].erase(friendid);
    friends_[friendid].erase(userid);
}

void FriendManager::SetFriendBlock(int userid, int friendid, bool block)
{
    friends_[userid][friendid]->SetBlock(block);
}

bool FriendManager::GetFriendBlock(int userid, int friendid)
{
    return friends_[userid][friendid]->GetBlock();
}

std::vector<int> FriendManager::GetFriendList(int userid)
{
    std::vector<int> friendlist;
    for (auto it : friends_[userid])
    {
        friendlist.push_back(it.first);
    }
    return friendlist;
}