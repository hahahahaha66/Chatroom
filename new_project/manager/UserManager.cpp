#include "UserManager.h"
#include <iostream>
#include <memory>
#include <unordered_map>

std::shared_ptr<User> UserManager::GetUser(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(userId);
    return it != users_.end() ? it->second : nullptr;
}

bool UserManager::AddUser(int id, const std::string& name, const std::string& password, const std::string& email) 
{

    std::lock_guard<std::mutex> lock(mutex_);

    if (emailtoidmap_.count(email)) {
        return false;  // 重复注册
    }

    if (id <= 0) return false;  // 插入失败

    auto user = std::make_shared<User>(id, name, password, email);
    users_[id] = user;
    emailtoidmap_[email] = id;
    idtoemailmap_[id] = email;
    return true;
}

void UserManager::RemoveUser(int id) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    users_.erase(id);
    std::string email = idtoemailmap_[id];
    emailtoidmap_.erase(email);
    idtoemailmap_.erase(id);
}

void UserManager::SetOnline(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(userId);
    if (it != users_.end()) {
        it->second->SetOnline(true);
    }
}

void UserManager::SetOffline(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(userId);
    if (it != users_.end()) {
        it->second->SetOnline(false);
    }
}

bool UserManager::IsOnline(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(userId);
    return (it != users_.end()) && it->second->IsOnline();
}

std::unordered_map<int, std::shared_ptr<User>> UserManager::GetAllUser()
{
    return users_;
}

std::unordered_map<std::string, int> UserManager::GetEmailToId()
{
    return emailtoidmap_;
}

std::unordered_map<int, std::string> UserManager::GetIdToEmail()
{
    return idtoemailmap_;
}