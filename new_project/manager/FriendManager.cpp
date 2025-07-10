#include "FriendManager.h"
#include <memory>
#include <vector>

bool FriendManager::AddFriend(int id, int userid, int friendid)
{
    if (userid == friendid) {
        return false;
    }

    if (IsFriend(userid, friendid))
    {
        return false;
    }

    friends_[userid][friendid] = std::make_shared<Friend>(id, userid, friendid);
    return true;
}

bool FriendManager::RemoveFriend(int userid, int friendid)
{
    if (!IsFriend(userid, friendid)) {
        return false;
    }

    friends_[userid].erase(friendid);
    friends_[friendid].erase(userid);
    return true;
}

bool FriendManager::SetFriendBlock(int userid, int friendid, bool block)
{
    if (!IsFriend(userid, friendid)) {
        return false;
    }

    friends_[userid][friendid]->SetBlock(block);
    return true;
}

bool FriendManager::GetFriendBlock(int userid, int friendid)
{
    if (!IsFriend(userid, friendid)) {
        return false;
    }
    return friends_[userid][friendid]->GetBlock();
}

bool FriendManager::IsFriend(int userid, int friendid)
{
    auto userit = friends_.find(userid);
    if (userit == friends_.end()) {
        return false;
    }
    
    return userit->second.find(friendid) != userit->second.end();
}

std::vector<int> FriendManager::GetFriendList(int userid)
{
    std::vector<int> friendlist;
    
    auto userit = friends_.find(userid);
    if (userit != friends_.end()) {
        for (const auto& friendPair : userit->second) {
            friendlist.push_back(friendPair.first);
        }
    }
    
    return friendlist;
}

std::vector<int> FriendManager::GetUnblockedFriendList(int userid)
{
    std::vector<int> friendlist;
    auto userit = friends_.find(userid);
    if (userit != friends_.end()) {
        for (const auto& friendPair : userit->second) {
            if (!friendPair.second->GetBlock()) {
                friendlist.push_back(friendPair.first);
            }
        }
    }
    
    return friendlist;
}