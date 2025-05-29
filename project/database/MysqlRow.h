#ifndef MYSQLROW_H
#define MYSQLROW_H

#include <cstdio>
#include <cstdlib>
#include <map>
#include <mysql/mysql.h>
#include <string>

class MysqlRow  
{
public:
    MysqlRow(MYSQL_ROW row, const std::map<std::string, int>& colmap)
        : row_(row), colmap_(colmap) {}
    
    std::string GetString(const std::string& field) const 
    {
        auto it = colmap_.find(field);
        if (it == colmap_.end() || !row_[it->second]) return 0;
        return row_[it->second];
    }

    int GetInt(const std::string& field) const 
    {
        auto it = colmap_.find(field);
        if (it == colmap_.end() || !row_[it->second]) return 0;
        return std::atoi(row_[it->second]);
    }
    
    bool GetBool(const std::string& field) const
    {
        auto it = colmap_.find(field);
        if (it == colmap_.end() || !row_[it->second]) return 0;
        std::string val = row_[it->second];
        return val == "1" || val == "true" || val == "TRUE";
    }

    tm GetDataTime(const std::string& field) const 
    {
        tm t = {};
        std::string val = GetString(field);

        if (val.empty()) return t;

        sscanf(val.c_str(), "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday,
               &t.tm_hour, &t.tm_min, &t.tm_sec);
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        return t;
    }

private:
    MYSQL_ROW row_;
    const std::map<std::string, int>& colmap_;
};

#endif