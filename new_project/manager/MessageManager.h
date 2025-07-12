#pragma one

#include "../model/Message.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <shared_mutex>
#include <mutex>

class MessageManager
{
public:
    //添加一条消息
    void AddMessage(int msgid, int senid, int recid, std::string content, const std::string type, const std::string status, std::string time);
    std::unordered_map<int, std::shared_ptr<Message>> GetAllMessage();

    std::vector<Message> GetSenderidAndReceiveridMessage(int senderid, int receiverid); // 查询

private:
    std::unordered_map<int, std::shared_ptr<Message>> messages_;
    std::unordered_map<int, std::unordered_map<int, std::unordered_set<int>>> senderidandreceiveridtomessage_;
    mutable std::shared_mutex mutex_;  
};