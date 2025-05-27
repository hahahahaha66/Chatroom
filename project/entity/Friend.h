#ifndef FRIEND_H
#define FRIEND_H

#include <string>

class Friend  
{
public:
    Friend();
    Friend(int userid, int friendid, std::string Status = "Normal");

    void SetUserId(int userid);
    int GetUserId();

    void SetFriendId(int friendid);
    int GetFriendId();

    void SetStatus(std::string status);
    std::string GetStatus();

private:
    enum class Status  
    {
        Normal,
        Block
    };

    int userid_;
    int friendid_;
    Status status_;
};

#endif