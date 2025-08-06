#include "MysqlConnection.h"

#include <chrono>
#include <iostream>
#include <mysql/mysql.h>

using namespace std::chrono;

MysqlConnection::MysqlConnection() { conn_ = mysql_init(nullptr); }

MysqlConnection::~MysqlConnection() {
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
}

bool MysqlConnection::Connect(const std::string &host, unsigned short port,
                              const std::string &user,
                              const std::string &password,
                              const std::string &name) {
    LOG_INFO << "Attempting to connect to MySQL at " << host << ": " << port
             << " with database: " << name;

    MYSQL *res =
        mysql_real_connect(conn_, host.c_str(), user.c_str(), password.c_str(),
                           name.c_str(), port, nullptr, 0);

    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    name_ = name;

    if (res) {
        LOG_INFO << "Successfully connected to MySQL server at " << host << ":"
                 << port;
        return true;
    } else {
        LOG_ERROR << "Failed to connect to MySQL server at " << host << ":"
                  << port << ": " << mysql_error(conn_);
        return false;
    }
}

bool MysqlConnection::ExcuteUpdate(const std::string &mysqlorder) {
    if (mysql_query(conn_, mysqlorder.c_str())) {
        LOG_ERROR << "Updata failed: " << mysql_error(conn_);
        return false;
    }

    my_ulonglong affected = mysql_affected_rows(conn_);
    if (affected == 0) {
        LOG_ERROR << "Update succeeded but no rows were affected.";
        return false; // 可根据你业务逻辑选择 true 或 false
    }
    return true;
}

MYSQL_RES *MysqlConnection::ExcuteQuery(const std::string &mysqlorder) {
    if (mysql_query(conn_, mysqlorder.c_str())) {
        LOG_ERROR << "Query failed: " << mysql_error(conn_);
        return nullptr;
    }
    return mysql_store_result(conn_);
}

MYSQL *MysqlConnection::GetRaw() { return conn_; }

void MysqlConnection::RefrushAliveTime() {
    lastUsedTime_ =
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
            .count();
}

long long MysqlConnection::GetLastTime() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
               .count() -
           lastUsedTime_;
}