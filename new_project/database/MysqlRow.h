#ifndef MYSQLROW_H
#define MYSQLROW_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mysql/mysql.h>
#include <string>
#include <strings.h>

class MysqlRow {
  public:
    MysqlRow(MYSQL_ROW row, const std::map<std::string, int> &colmap)
        : row_(row), colmap_(colmap) {}

    std::string GetString(const std::string &field) const {
        auto it = colmap_.find(field);
        if (it == colmap_.end())
            return "";
        const char *val = row_[it->second];
        return val ? std::string(val) : "";
    }

    int GetInt(const std::string &field) const {
        auto it = colmap_.find(field);
        if (it == colmap_.end())
            return 0;
        const char *val = row_[it->second];
        return val ? std::atoi(val) : 0;
    }

    bool GetBool(const std::string &field) const {
        auto it = colmap_.find(field);
        if (it == colmap_.end())
            return false;
        const char *val = row_[it->second];
        if (!val)
            return false;
        return std::strcmp(val, "1") == 0 || strcasecmp(val, "true") == 0;
    }

    tm GetDataTime(const std::string &field) const {
        tm t = {};
        std::string val = GetString(field);
        if (val.empty())
            return t;

        sscanf(val.c_str(), "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon,
               &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        return t;
    }

    MYSQL_ROW Get() { return row_; }

  private:
    MYSQL_ROW row_;
    const std::map<std::string, int> &colmap_;
};

#endif