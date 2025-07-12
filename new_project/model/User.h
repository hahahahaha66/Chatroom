#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include <string>

class User 
{
public:
    User(int id, const std::string& name, const std::string& password, const std::string& email)
        : id_(id), name_(name), password_(password), email_(email), online_(false) {}

    User(const User&) = delete;
    User& operator=(const User&) = delete;

    User(User&& other) noexcept
        : id_(other.id_),
          name_(std::move(other.name_)),
          password_(std::move(other.password_)),
          email_(std::move(other.email_)),
          online_(other.online_),
          conn_(std::move(other.conn_))
    {
        // mutex_ 不动，保留默认状态
    }     

    User& operator=(User&& other) noexcept
    {
        if (this != &other)
        {
            id_ = other.id_;
            name_ = std::move(other.name_);
            password_ = std::move(other.password_);
            email_ = std::move(other.email_);
            online_ = other.online_;
            conn_ = std::move(other.conn_);
        }
        return *this;
    }

    int GetId() const 
    { 
        return id_; 
    }
    std::string GetName() const 
    {
        return name_; 
    }
    std::string GetPassword() const 
    { 
        return password_; 
    }
    std::string GetEmail() const 
    { 
        return email_; 
    }
    TcpConnectionPtr GetConn() const 
    {
        return conn_; 
    }

    void SetConn(TcpConnectionPtr conn) 
    { 
        conn_ = conn; 
    }
    void SetOnline(bool online) 
    { 
        online_ = online; 
    }
    bool IsOnline() const 
    {
        return online_; 
    }

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string email_;
    bool online_;
    TcpConnectionPtr conn_;
};