#pragma one

class Friend 
{
public:
    Friend(int userid, int friendid)
        : userid_(userid), friendid_(friendid)
    {
    }

    int GetUserId() { return userid_; }
    int GetFriendId() { return friendid_; }

    bool GetBlock() { return block_; }
    void SetBlock(bool block) { block_ = block; }

private:
    int userid_;
    int friendid_;
    bool block_ = false;
};