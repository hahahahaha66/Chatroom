#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <ctime>
#include <string>

class GroupUser  
{
public:
    GroupUser();
    GroupUser(int userid, const std::string& username, const std::string& role);

    int GetUserId() const;
    const std::string& GetUserName() const;
    const std::string& GetRole() const;
    bool IsMuted() const;
    time_t GetJoinTime() const;
    time_t GetLastReadTimre() const;

    void SetRole(const std::string& role);
    void SetMuted(bool muted);
    void SetLastReadTime(time_t lastreadtime);

private: 
    int userid_;
    std::string username_;
    std::string role_;
    bool muted_ = false;
    time_t jointime_;
    time_t lastreadtime_;
};

#endif