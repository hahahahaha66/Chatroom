#ifndef MYSQLCONNECTIONPOOL_H
#define MYSQLCONNECTIONPOOL_H

#include "../muduo/logging/Logging.h"
#include "MysqlConnection.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using MysqlConnPtr = std::shared_ptr<MysqlConnection>;

class MysqlConnectionPool {
  public:
    static MysqlConnectionPool &Instance();

    void Init(const std::string &host, unsigned short port,
              const std::string &user, const std::string &password,
              const std::string &name, int maxsize);

    std::shared_ptr<MysqlConnection> GetConnection();

  private:
    MysqlConnectionPool();
    ~MysqlConnectionPool();

    void ProduceConnectionTask();
    void RecycleMysqlConnectionTask();

    std::string host_;
    std::string user_;
    std::string password_;
    std::string name_;

    unsigned short port_;
    int maxsize_;

    std::queue<MysqlConnection *> connectionqueue_;
    std::mutex mutex_;
    std::condition_variable cond_;

    bool isrunning_;
};

#endif