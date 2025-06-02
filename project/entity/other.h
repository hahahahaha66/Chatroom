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

struct Message
{
    enum class Type {Private, Group};
    enum class Status {read, unread};

    Type type_;
    int senderid_;
    int receiverid_;
    std::string connnect_;
    Status status_;
};

#endif