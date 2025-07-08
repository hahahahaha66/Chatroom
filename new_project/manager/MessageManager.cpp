#include "MessageManager.h"
#include <memory>
#include <mutex>

void MessageManager::AddMessage(Message& message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    messages_[message.GetId()] = std::make_shared<Message>(message);
}

std::vector<Message> MessageManager::GetSenderidAndReceiveridMessage(int senderid, int receiverid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<Message> messages;
    for (int messageid : senderidandreceiveridtomessage_[senderid][receiverid])
    {
        auto it = messages_[messageid];
        Message msg(messageid, it->GetSenderId(), it->GetReveiverId(), it->GetContent(), it->GetType(), it->GetStatus(), it->GetTime());

        messages.push_back(msg);
    }

    return messages;
}