#include "MessageManager.h"
#include <memory>
#include <mutex>
#include <unordered_map>

void MessageManager::AddMessage(int msgid, int senid, int recid, std::string content, const std::string type, const std::string status, std::string time)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    messages_[msgid] = std::make_shared<Message>(msgid, senid, recid, content, type, status, time);
}

std::unordered_map<int, std::shared_ptr<Message>> MessageManager::GetAllMessage()
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return messages_;
}

std::vector<Message> MessageManager::GetSenderidAndReceiveridMessage(int senderid, int receiverid)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    std::vector<Message> messages;
    for (int messageid : senderidandreceiveridtomessage_[senderid][receiverid])
    {
        auto it = messages_[messageid];
        Message msg(messageid, it->GetSenderId(), it->GetReveiverId(), it->GetContent(), it->GetType(), it->GetStatus(), it->GetTime());

        messages.push_back(std::move(msg));
    }

    return messages;
}
