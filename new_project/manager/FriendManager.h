#pragma one

#include "../model/Friend.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

class FriendManager 
{
public:
    //记得添加双向好友
    bool AddFriend(int id, int userid, int friendid, bool block = false);
    bool RemoveFriend(int useid, int friendid);
    bool SetFriendBlock(int userid, int friendid, bool block);
    bool GetFriendBlock(int userid, int friendid);
    bool IsFriend(int userid, int friendid);

    std::vector<std::shared_ptr<Friend>> GetFriendList(int userid);
    std::vector<std::shared_ptr<Friend>> GetUnblockedFriendList(int userid);
    size_t GetFriendCount(int userid);
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Friend>>> GetAllFriend();

private:
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Friend>>> friends_;
    std::mutex mutex_;
};