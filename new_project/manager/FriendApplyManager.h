#pragma one

#include "../model/Apply.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>

class FriendApplyManager
{
public:
    bool AddAplly(int id, int fromid, int targetid, std::string type = "Friend", std::string status = "Pending");

    bool AcceptApply(int fromid, int targetid);
    bool RejectApply(int fromid, int targetid);

    bool HasApply(int fromId, int targetId);
    bool DeleteApply(int fromid, int targetid);

    std::shared_ptr<Apply> GetApply(int fromid, int targetid);
    //获取受到的申请列表
    std::vector<std::shared_ptr<Apply>> GetReceivedApplies(int targetid);
    //获取发出的申请列表
    std::vector<std::shared_ptr<Apply>> GetSentApplies(int fromid);
    //获取待处理的申请列表
    std::vector<std::shared_ptr<Apply>> GetPendingApplies(int targetid);
    //获取申请状态
    std::string GetApplyStatus(int fromId, int targetId);
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Apply>>> GetAllApply();
    
private:
    //以被发送申请人作为索引,[target][from]->apply
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<Apply>>> applys;
    std::mutex mutex_;
};