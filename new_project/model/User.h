#pragma once

#include "../muduo/net/tcp/TcpConnection.h"
#include <string>

class User {
  public:
    User(int id, const std::string &name, const std::string &password,
         const std::string &email)
        : id_(id), name_(name), password_(password), email_(email),
          online_(false), istransferfiles_(false), userinterfaceid(0),
          timeout_(0), userinterface_("") {}
    User() = default;

    int GetId() const { return id_; }
    std::string GetName() const { return name_; }
    std::string GetPassword() const { return password_; }
    std::string GetEmail() const { return email_; }
    int GetUserInterFaceId() const { return userinterfaceid; }
    std::string GetUserInterFace() const { return userinterface_; }
    TcpConnectionPtr GetConn() const { return conn_; }

    void SetUserInterFaceId(int id) { userinterfaceid = id; }
    void SetUserInterFace(std::string interface) { userinterface_ = interface; }
    void SetConn(TcpConnectionPtr conn) { conn_ = conn; }
    void SetOnline(bool online) { online_ = online; }
    bool IsOnline() const { return online_; }

    void SetTimeOut(int timeout) { timeout_ = timeout; }
    void TimeOutIncrement() { timeout_++; }
    int GetTimeOut() { return timeout_; }
    void SetIsTransferFiles(bool status) { istransferfiles_ = status; }
    int GetIsTransferFiles() { return istransferfiles_; }

  private:
    int id_;
    std::string name_;
    std::string password_;
    std::string email_;
    std::string userinterface_;
    int userinterfaceid;
    bool online_;
    bool istransferfiles_;
    int timeout_;
    TcpConnectionPtr conn_;
};