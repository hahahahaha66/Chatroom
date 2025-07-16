#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include <string>

class User 
{
public:
    User(int id, const std::string& name, const std::string& password, const std::string& email)
        : id_(id), name_(name), password_(password), email_(email), online_(false), userinterfaceid(0) {}

    int GetId() const { return id_; }
    std::string GetName() const { return name_; }
    std::string GetPassword() const { return password_; }
    std::string GetEmail() const { return email_; }
    int GetUserInterface() const { return userinterfaceid; }
    TcpConnectionPtr GetConn() const { return conn_; }

    void SetUserInterface(int id) { userinterfaceid = id; } 
    void SetConn(TcpConnectionPtr conn) { conn_ = conn; }
    void SetOnline(bool online) { online_ = online; }
    bool IsOnline() const { return online_; }

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string email_;
    int userinterfaceid;
    bool online_;
    TcpConnectionPtr conn_;
};