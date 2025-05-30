#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <ctime>
#include <string>
#include <stdexcept>

class GroupUser  
{
public:
    enum class Level
    {
        Group_owner,
        Administrator,
        Member
    };
    
    GroupUser();
    GroupUser(int userid, const std::string& role);

    int GetUserId() const;
    const std::string& GetUserName() const;
    const std::string GetRole() const;
    bool IsMuted() const;
    
    time_t GetJoinTime() const;
    time_t GetLastReadTimre() const;

    void SetRole(const std::string& role);
    void SetMuted(bool muted);
    void SetLastReadTime(time_t lastreadtime);

private: 
    int userid_;
    Level role_;
    bool muted_ = false;  //禁言

    time_t jointime_;
    time_t lastreadtime_;
};

#endif