#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include "../muduo/logging/Logging.h"

#include <mysql/mysql.h>
#include <string>

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    bool Connect(const std::string& host, unsigned short port, const std::string& user,
                 const std::string& password, const std::string& name);
    
    bool ExcuteUpdate(const std::string& mysqlorder);
    MYSQL_RES* ExcuteQuery(const std::string& mysqlorder);

    MYSQL* GetRaw();

    void RefrushAliveTime();
    long long GetLastTime();

    bool IsConnected() {
        return mysql_ping(conn_) == 0;
    }

    bool Reconnect() {
        mysql_close(conn_);
        conn_ = mysql_init(nullptr);
        return Connect(host_, port_, user_, password_, name_);
    }

private:
    std::string host_, user_, password_, name_;
    unsigned short port_;

    MYSQL* conn_;
    long long lastUsedTime_;
};

#endif