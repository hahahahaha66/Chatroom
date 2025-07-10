#pragma one

#include "../model/Friend.h"

#include <vector>
#include <unordered_map>
#include <memory>

class FriendManager 
{
public:
    //记得添加双向好友
    bool AddFriend(int id, int userid, int friendid);
    bool RemoveFriend(int useid, int friendid);
    bool SetFriendBlock(int userid, int friendid, bool block);
    bool GetFriendBlock(int userid, int friendid);
    bool IsFriend(int userid, int friendid);

    std::vector<int> GetFriendList(int userid);
    std::vector<int> GetUnblockedFriendList(int userid);
    size_t GetFriendCount(int userid);

private:
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Friend>>> friends_;
};