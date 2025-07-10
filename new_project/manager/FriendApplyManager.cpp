#include "FriendApplyManager.h"

bool FriendApplyManager::AddAplly(int id, int fromid, int targetid, std::string type, std::string status)
{
    if (HasApply(fromid, targetid))
    {
        return false;
    }

    applys[targetid][fromid] = std::make_shared<Apply>(id, fromid, targetid, type, status);
    return true;
}

bool FriendApplyManager::AcceptApply(int fromid, int targetid)
{
    auto it = applys.find(targetid);
    if (it == applys.end())
    {
        return false;
    }

    auto applyit = it->second.find(fromid);
    if (applyit == it->second.end())
    {
        return false;
    }

    applyit->second->SetStatus("Accept");
    return true;
}

bool FriendApplyManager::RejectApply(int fromid, int targetid)
{
    auto it = applys.find(targetid);
    if (it == applys.end())
    {
        return false;
    }

    auto applyit = it->second.find(fromid);
    if (applyit == it->second.end())
    {
        return false;
    }

    applyit->second->SetStatus("Reject");
    return true;
}

bool FriendApplyManager::DeleteApply(int fromid, int targetid)
{
    auto it = applys.find(targetid);
    if (it == applys.end())
    {
        return false;
    }

    auto applyit = it->second.find(fromid);
    if (applyit == it->second.end())
    {
        return false;
    }

    applys[fromid].erase(fromid);

    if (it->second.empty()) {
        applys.erase(it);
    }

    return true;
}

std::shared_ptr<Apply> FriendApplyManager::GetApply(int fromid, int targetid)
{
    auto it = applys.find(targetid);
    if (it == applys.end())
    {
        return nullptr;
    }

    auto applyit = it->second.find(fromid);
    if (applyit == it->second.end())
    {
        return nullptr;
    }

    return applyit->second;
}

std::vector<std::shared_ptr<Apply>> FriendApplyManager::GetReceivedApplies(int targetid)
{
    std::vector<std::shared_ptr<Apply>> result;
    
    auto it = applys.find(targetid);
    if (it != applys.end()) {
        for (const auto& pair : it->second) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Apply>> FriendApplyManager::GetSentApplies(int fromid)
{
    std::vector<std::shared_ptr<Apply>> result;
    
    for (const auto& targetPair : applys) {
        for (const auto& applyPair : targetPair.second) {
            if (applyPair.second->GetFromId() == fromid) {
                result.push_back(applyPair.second);
            }
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Apply>> FriendApplyManager::GetPendingApplies(int targetid)
{
    std::vector<std::shared_ptr<Apply>> result;
    
    auto it = applys.find(targetid);
    if (it != applys.end()) {
        for (const auto& pair : it->second) {
            if (pair.second->GetStatus() == "Pending") {
                result.push_back(pair.second);
            }
        }
    }
    
    return result;
}

bool FriendApplyManager::HasApply(int fromid, int targetid)
{
    auto it = applys.find(targetid);
    if (it == applys.end()) {
        return false;
    }
    
    return it->second.find(fromid) != it->second.end();
}

std::string FriendApplyManager::GetApplyStatus(int fromid, int targetid)
{
    auto apply = GetApply(fromid, targetid);
    if (!apply) {
        return "Pending"; // 默认状态
    }
    
    return apply->GetStatus();
}