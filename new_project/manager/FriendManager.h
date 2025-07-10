#pragma one

#include "../model/Friend.h"

#include <vector>
#include <unordered_map>
#include <memory>

class FriendManager 
{
public:
    void AddFriend(int userid, int friendid);
    void RemoveFriend(int useid, int friendid);
    void SetFriendBlock(int userid, int friendid, bool block);
    bool GetFriendBlock(int userid, int friendid);

    std::vector<int> GetFriendList(int userid);

private:
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Friend>>> friends_;
};