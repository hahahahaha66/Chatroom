#include "UserManager.h"
#include <iostream>

// 模拟数据库操作（你应该用自己的 Database 类封装）
int insertUserIntoDB(const std::string& name) {
    // 模拟插入数据库并返回数据库分配的ID
    static int nextId = 1000;
    std::cout << "[DB] Insert user: " << name << ", assigned id = " << nextId << "\n";
    return nextId++;  // 实际项目里从数据库中获取插入后返回的 ID
}

void UserManager::LoadAllUsersFromDB() 
{
    // 假设从数据库中读取到3个用户
    std::vector<std::pair<int, std::string>> dbUsers = {
        {1001, "Alice"}, {1002, "Bob"}, {1003, "Charlie"}
    };

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [id, name] : dbUsers) {
        auto user = std::make_shared<User>(id, name);
        users_[id] = user;
        nametoidmap_[name] = id;
        idtonamemap_[id] = name;
    }
}

std::shared_ptr<User> UserManager::GetUser(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(userId);
    return it != users_.end() ? it->second : nullptr;
}

bool UserManager::addUserToSystem(const std::string& name, const std::string& password, const std::string& email) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (nametoidmap_.count(name)) {
        return false;  // 重复注册
    }

    int id = insertUserIntoDB(name);
    if (id <= 0) return false;  // 插入失败

    auto user = std::make_shared<User>(id, name);
    users_[id] = user;
    nametoidmap_[name] = id;
    idtonamemap_[id] = name;
    return true;
}

void UserManager::RemoveUser(int userId) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    users_.erase(userId);
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
