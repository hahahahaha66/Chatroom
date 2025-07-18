#include "FriendManager.h"
#include <memory>
#include <vector>

bool FriendManager::AddFriend(int id, int userid, int friendid, bool block)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (userid == friendid) {
        return false;
    }

    if (IsFriend(userid, friendid))
    {
        return false;
    }

    friends_[userid][friendid] = std::make_shared<Friend>(id, userid, friendid, block);
    return true;
}

bool FriendManager::RemoveFriend(int userid, int friendid)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!IsFriend(userid, friendid)) {
        return false;
    }

    friends_[userid].erase(friendid);
    friends_[friendid].erase(userid);
    return true;
}

bool FriendManager::SetFriendBlock(int userid, int friendid, bool block)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!IsFriend(userid, friendid)) {
        return false;
    }

    friends_[userid][friendid]->SetBlock(block);
    return true;
}

bool FriendManager::GetFriendBlock(int userid, int friendid)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!IsFriend(userid, friendid)) {
        return false;
    }
    return friends_[userid][friendid]->GetBlock();
}

bool FriendManager::IsFriend(int userid, int friendid)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto userit = friends_.find(userid);
    if (userit == friends_.end()) {
        return false;
    }
    
    return userit->second.find(friendid) != userit->second.end();
}

std::vector<std::shared_ptr<Friend>> FriendManager::GetFriendList(int userid)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    std::vector<std::shared_ptr<Friend>> friendlist;
    
    auto userit = friends_.find(userid);
    if (userit != friends_.end()) {
        for (const auto& friendPair : userit->second) {
            friendlist.push_back(friendPair.second);
        }
    }
    
    return friendlist;
}

std::vector<std::shared_ptr<Friend>> FriendManager::GetUnblockedFriendList(int userid)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    std::vector<std::shared_ptr<Friend>> friendlist;
    auto userit = friends_.find(userid);
    if (userit != friends_.end()) {
        for (const auto& friendPair : userit->second) {
            if (!friendPair.second->GetBlock()) {
                friendlist.push_back(friendPair.second);
            }
        }
    }
    
    return friendlist;
}

std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Friend>>> FriendManager::GetAllFriend()
{
   std::shared_lock<std::shared_mutex> lock(mutex_);

    return friends_;
}