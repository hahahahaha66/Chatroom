#ifndef OTHER_H
#define OTHER_H

#include <string>

struct ChatConnect 
{
    enum class Type {None, Private, Group};
    Type type_;
    int peerid_;
    ChatConnect(int peerid, Type type = Type::None) : type_(type), peerid_(peerid) {}
};

struct SimpUser
{
    int userid_;
    std::string username_;
    SimpUser(int userid) : userid_(userid), username_(nullptr) {}
    SimpUser(int userid, std::string username) : userid_(userid), username_(username) {}
};

struct Message
{
    enum class Type {Private, Group};
    enum class Status {read, unread};

    Type type_;
    int senderid_;
    int receiverid_;
    std::string connect_;
    Status status_;
    Message(int senderid, int receiverid, std::string connect)
        : senderid_(senderid),
          receiverid_(receiverid),
          connect_(connect)
    {
    }

    Message(int senderid, int receiverid, std::string connect, Status, Type)
        : senderid_(senderid),
          receiverid_(receiverid),
          connect_(connect)
    {
    }
};

#endif