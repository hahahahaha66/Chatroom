#pragma once

#include "../model/User.h"
#include "../muduo/logging/Logging.h"

#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include <vector>
#include <shared_mutex>


class UserManager 
{
public:
    std::shared_ptr<User> GetUser(int userId); // 获取用户信息
    void RemoveUser(int userId);               // 注销用户

    bool AddUser(int id, const std::string& name, const std::string& password, const std::string& email);

    void SetOnline(int userId);  // 登录
    void SetOffline(int userId); // 下线
    bool IsOnline(int userId);

    std::unordered_map<int, std::shared_ptr<User>> GetAllUser();
    std::unordered_map<std::string, int> GetEmailToId();
    std::unordered_map<int, std::string> GetIdToEmail();

private:
    std::unordered_map<int, std::shared_ptr<User>> users_; // 所有用户
    std::unordered_map<std::string, int> emailtoidmap_;
    std::unordered_map<int, std::string> idtoemailmap_;
    mutable std::shared_mutex mutex_;
};
