#pragma once

#include "../model/User.h"

#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include <vector>


class UserManager 
{
public:
    void LoadAllUsersFromDB();  // 初始化时调用

    std::shared_ptr<User> GetUser(int userId); // 获取用户信息
    void RemoveUser(int userId);               // 注销用户

    bool addUserToSystem(const std::string& name, const std::string& password, const std::string& email, TcpConnectionPtr conn);

    void SetOnline(int userId);  // 登录
    void SetOffline(int userId); // 下线
    bool IsOnline(int userId);

    std::unordered_map<std::string, int> GetEmailToId();
    std::unordered_map<int, std::string> GetIdToEmail();

private:
    std::unordered_map<int, std::shared_ptr<User>> users_; // 所有用户
    std::unordered_map<std::string, int> emailtoidmap_;
    std::unordered_map<int, std::string> idtoemailmap_;
    std::mutex mutex_;
};
