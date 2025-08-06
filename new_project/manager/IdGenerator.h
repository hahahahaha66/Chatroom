#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <sw/redis++/connection.h>
#include <sw/redis++/redis.h>

class IdGenerator {
  public:
    IdGenerator() {
        sw::redis::ConnectionOptions opts;
        opts.host = "127.0.0.1";
        opts.port = 6379;

        try {
            redis_ = std::make_shared<sw::redis::Redis>(opts);
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to connect to Redis: " +
                                     std::string(e.what()));
        }
    }

    int GetNextUserId() {
        try {
            return redis_->incr("global:userid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment userid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextMsgId() {
        try {
            return redis_->incr("global:msgid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment msgid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextFriendId() {
        try {
            return redis_->incr("global:friendid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment friendid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextGroupId() {
        try {
            return redis_->incr("global:groupid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment groupid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextGroupUserId() {
        try {
            return redis_->incr("global:groupuserid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment groupuserid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextFriendApplyId() {
        try {
            return redis_->incr("global:friendapplyid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment friendapplyid: " +
                                     std::string(e.what()));
        }
    }

    int GetNextGroupApplyId() {
        try {
            return redis_->incr("global:groupapplyid");
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to increment groupapplyid: " +
                                     std::string(e.what()));
        }
    }

    void InitUserId(int startid) {
        try {
            redis_->set("global:userid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init userid: " +
                                     std::string(e.what()));
        }
    }

    void InitMsgId(int startid) {
        try {
            redis_->set("global:msgid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init msgid: " +
                                     std::string(e.what()));
        }
    }

    void InitFriendId(int startid) {
        try {
            redis_->set("global:friendid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init friendid: " +
                                     std::string(e.what()));
        }
    }

    void InitGroupId(int startid) {
        try {
            redis_->set("global:groupid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init groupid: " +
                                     std::string(e.what()));
        }
    }

    void InitGroupUserId(int startid) {
        try {
            redis_->set("global:groupuserid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init groupuserid: " +
                                     std::string(e.what()));
        }
    }

    void InitFriendApplyId(int startid) {
        try {
            redis_->set("global:friendapplyid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init friendapplyid: " +
                                     std::string(e.what()));
        }
    }

    void InitGroupApplyId(int startid) {
        try {
            redis_->set("global:groupapplyid", std::to_string(startid));
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to init groupapplyid: " +
                                     std::string(e.what()));
        }
    }

    std::shared_ptr<sw::redis::Redis> GetRedis() { return redis_; }

  private:
    std::shared_ptr<sw::redis::Redis> redis_;
};