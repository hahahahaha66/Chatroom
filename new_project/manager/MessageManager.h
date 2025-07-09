#pragma one

#include "../model/Message.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <mutex>

class MessageManager
{
public:
    void AddMessage(Message& message); //添加一条消息
    std::unordered_map<int, std::shared_ptr<Message>> GetAllMessage();

    std::vector<Message> GetSenderidAndReceiveridMessage(int senderid, int receiverid); // 查询

private:
    std::unordered_map<int, std::shared_ptr<Message>> messages_;
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> senderidandreceiveridtomessage_;
    std::mutex mutex_;
};