#ifndef MYSQLRESULT_H
#define MYSQLRESULT_H

#include "MysqlRow.h"

#include <map>
#include <mysql/mysql.h>
#include <stdexcept>
#include <string>

class MysqlResult {
  public:
    MysqlResult(MYSQL_RES *res) : res_(res) {
        int num_fields = mysql_num_fields(res_);
        MYSQL_FIELD *fields = mysql_fetch_fields(res_);
        for (int i = 0; i < num_fields; i++) {
            colmap_[fields[i].name] = i;
        }
    }

    ~MysqlResult() {
        if (res_)
            mysql_free_result(res_);
    }

    bool Next() {
        currentrow_ = mysql_fetch_row(res_);
        return currentrow_ != nullptr;
    }

    MysqlRow GetRow() const {
        if (!currentrow_)
            throw std::runtime_error(
                "Attempt to access row before calling Next()");
        return MysqlRow(currentrow_, colmap_);
    }

  private:
    MYSQL_RES *res_;
    MYSQL_ROW currentrow_;
    std::map<std::string, int> colmap_;
};

#endif