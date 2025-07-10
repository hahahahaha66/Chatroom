#pragma one

class Friend 
{
public:
    Friend(int id, int userid, int friendid, bool block)
        : id_(id), userid_(userid), friendid_(friendid), block_(block)
    {
    }

    int GetId() { return id_; }
    int GetUserId() { return userid_; }
    int GetFriendId() { return friendid_; }

    bool GetBlock() { return block_; }
    void SetBlock(bool block) { block_ = block; }

private:
    int id_;
    int userid_;
    int friendid_;
    bool block_ = false;
};