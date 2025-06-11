#ifndef FRIEND_H
#define FRIEND_H

#include <string>

class Friend  
{
public:
    Friend();
    Friend(int userid, int friendid, std::string Status = "Normal");
    Friend(int userid, std::string friendname, int friendid, std::string Status = "Normal");

    void SetUserId(int userid);
    int GetUserId() const;

    void SetFriendName(const std::string friendname);
    std::string GetFriendName() const;

    void SetOnline(bool online);
    bool GetOnline() const;

    void SetFriendId(int friendid);
    int GetFriendId() const;

    void SetStatus(std::string status);
    std::string GetStatus() const;

    void SetBlocked(bool block);
    bool GetBlocked() const; 

private:
    enum class Status  
    {
        Normal,
        Block
    };

    std::string friendname_;
    bool online_ = false;
    bool blocked_ = false;
    int userid_;
    int friendid_;
    Status status_;
};

#endif