#pragma one

class Friend 
{
public:
    Friend(int id, int userid, int friendid, bool block)
        : id_(id), userid_(userid), friendid_(friendid), block_(block) {}

    Friend(const Friend&) = delete;
    Friend& operator=(const Friend&) = delete;

    Friend(Friend&& other) noexcept
        : id_(other.id_),
          userid_(other.userid_),
          friendid_(other.friendid_),
          block_(other.block_)
    {
        // mutex_ 不动，保留默认状态
    }

    Friend& operator=(Friend&& other) noexcept
    {
        if (this != &other)
        {
            id_ = other.id_;
            userid_ = other.userid_;
            friendid_ = other.friendid_;
            block_ = other.block_;
        }
        return *this;
    }

    int GetId() 
    { 
        return id_; 
    }
    int GetUserId() 
    { 
        return userid_; 
    }
    int GetFriendId() 
    { 
        return friendid_; 
    }
    bool GetBlock() 
    { 
        return block_; 
    }

    void SetBlock(bool block) 
    { 
        block_ = block; 
    }

private:
    int id_;
    int userid_;
    int friendid_;
    bool block_ = false;
};