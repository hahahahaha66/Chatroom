#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <mysql/mysql.h>
#include <string>

class MysqlConnection {
public:
    MysqlConnection();
    ~MysqlConnection();

    bool Connect(const std::string& host, unsigned short port, const std::string& user,
                 const std::string& password, const std::string& name);
    
    bool ExcuteUpdata(const std::string& mysqlorder);
    MYSQL_RES* ExcuteQuery(const std::string& mysqlorder);

    MYSQL* GetRaw();

    void RefrushAliveTime();
    long long GetLastTime();

private:
    MYSQL* conn_;
    long long lastUsedTime_;

};

#endif