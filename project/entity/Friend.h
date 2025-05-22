#ifndef FRIEND_H
#define FRIEND_H

#include <string>

class Friend  
{
public:
    Friend();
    Friend(int friendid, std::string friendname, bool online);

    void SetFriendId(int& friendid);
    const int GetFriendId() const;

    void SetFriendName(std::string& friendname);
    const std::string& GetFriendName() const;

    void SetOnline(bool online);
    const bool GetOnline() const;

private:
    int friendid_;
    std::string friendname_;
    bool online_;
};

#endif