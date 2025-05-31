#include "Service.h"
#include "../json_protocol.hpp"
#include <cstdint>
#include <functional>
#include <future>
#include <mysql/mariadb_com.h>
#include <mysql/mysql.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>

Service::Service(Dispatcher& dispatcher) : dispatcher_(dispatcher)
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
    handermap_[2] = std::bind(&Service::RegisterAccount, this, _1, _2, _3, _4);
    handermap_[5] = std::bind(&Service::ListFriendlist, this, _1, _2, _3, _4);
    handermap_[7] = std::bind(&Service::DeleteFriend, this, _1, _2, _3, _4);
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
}

void Service::RegisterAllHanders(Dispatcher& dispatcher)
{
    for (auto& [id, hander] : handermap_)
    {
        dispatcher.registerHander(id, hander);
    }
}

void Service::ReadFromDataBase(const std::string& query, std::function<void(MysqlRow&)> rowhander)
{
    databasethreadpool_.EnqueueTask([this, query, rowhander](MysqlConnection& conn) {
        MYSQL_RES* res = conn.ExcuteQuery(query);
        if (!res) 
        {
            return;
        }

        MysqlResult result(res);
        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            rowhander(row);
        }
    });
}

void Service::ReadUserFromDataBase()
{
    std::string query = "select * from user;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int id = row.GetInt("id");
        std::string username = row.GetString("username");
        std::string password = row.GetString("password");
        bool online = row.GetBool("online");
        userlist_[id] = std::move(User(id, username, password));
    });
}

void Service::ReadGroupFromDataBase()
{
    std::string query = "select * from groups;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int id = row.GetInt("id");
        std::string groupname = row.GetString("groupname");
        grouplist_[id] = std::move(Group(id, groupname));
    });
}

void Service::ReadFriendFromDataBase()
{
    std::string query = "select * from friend;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int userid = row.GetInt("userid");
        int friendid = row.GetInt("friendid");
        std::string status = row.GetString("Status");

        userlist_[userid].AddFriend(friendid);
        userlist_[userid].SetStatusFriend(friendid, status);
        userfriendlist[userid][friendid] = std::move(Friend(userid, friendid, status));
    });
}

void Service::ReadChatConnectFromDataBase()
{
    std::string query = "select * from chatconnect;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int userid = row.GetInt("userid");
        ChatConnect::Type type;
        std::string str = row.GetString("type");
        if (str == "Private")
            type = ChatConnect::Type::Private;
        else if (str == "Group")
            type = ChatConnect::Type::Group;
        else    
            type = ChatConnect::Type::None;

        int peerid = row.GetInt("peerid");
        chatconnect_[userid] = std::move(ChatConnect(peerid, type));
    });
}

void Service::ReadGroupApplyFromDataBase()
{
    std::string query = "select * from groupapply;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int groupid = row.GetInt("groupid");
        int applyid = row.GetInt("applyid");
        grouplist_[groupid].AddApply(applyid);
    });
}

void Service::ReadUserApplyFromDataBase()
{
    std::string query = "select * from userapply;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int userid = row.GetInt("userid");
        int applyid = row.GetInt("applyid");
        userlist_[userid].AddApply(applyid);
    });
}

void Service::ReadGroupUserFromDataBase()
{
    std::string query = "select * from groupuser;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int userid = row.GetInt("id");
        int groupid = row.GetInt("groupid");
        std::string str = row.GetString("level");
        grouplist_[groupid].AddMember(std::move(GroupUser(userid, str)));
    });
}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::unordered_map<int, std::string> friendlist;
    std::unordered_map<int, std::string> grouplist;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (end == true)
    {
        if (userlist_[userid].GetPassWord() == password)
            end = true;
        else  
            end = false;
    }
    
    uint16_t type = (end == true ? 1 : 0);
    if (end) 
        userlist_[userid].SetOnline(conn);

    if (end)
    {
        for (auto& it : userlist_[userid].GetFriendList())
        {
            friendlist[it.first] = userlist_[it.first].GetUserName();
        }

        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist[it] = grouplist_[it].GetGroupName();
        }
    }

    json all_friend = js_AllFriendIdName(userid, friendlist, grouplist);
    conn->send(codec_.encode(all_friend, type, seq));
}


void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    std::promise<int> promise;
    std::future<int> result_future = promise.get_future();

    if (end)
    {
        //检查数据库中有无已创建的账号，有返回false，没有则写入返回true
        databasethreadpool_.EnqueueTask([username, password, p = std::move(promise)](MysqlConnection& conn)mutable {
            std::string check_sql = 
                "select exists (select 1 from user where username = '" + username + "' ) as user_exists;";
            MYSQL_RES* res = conn.ExcuteQuery(check_sql);
            if (!res) {
                p.set_value(-2);
                return ;
            }

            MysqlResult result(res);
            if (result.Next())
            {
                MysqlRow row = result.GetRow();
                bool exists = row.GetBool("user_exixts");
                if (exists)
                {
                    p.set_value(-1);
                    return;
                }
            }
            
            std::string insert_sql = 
                "insert into user (username, password) value ('" + username + "' , '" + password + "');";
            if (!conn.ExcuteQuery(insert_sql)) 
            {
                p.set_value(-3);
                return;
            }

            MYSQL_RES* idres = conn.ExcuteQuery("select last_insert_id() as is");
            if (!idres)
            {
                p.set_value(-4);
                return;
            }

            MysqlResult id_result(idres);
            if (id_result.Next())
            {
                MysqlRow row = id_result.GetRow();
                int id = row.GetInt("id");
                p.set_value(id);
            }
            else  
            {
                p.set_value(-5);
            }
        });
    }

    int userid = result_future.get();
    
    if (userid < 0)
        end = false;

    if (end)
        result = "Register successful!";
    else  
        result = "Register failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);
    if (end)
    {
        userlist_[userid] = {userid, username, password};
    }
       
    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendlist(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    
    std::unordered_map<int, bool> friendlist;
    for (auto& it : userfriendlist[userid])
    {
        friendlist[it.first] = userlist_[it.first].IsOnLine();
    }

    json reply_js = js_FriendList(userid, friendlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    end &= userlist_[userid].DeleteFriend(friendid);
    userfriendlist[userid].erase(friendid);

    if (end)
        result = "Delete successful!";
    else  
        result = "Delete failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::BlockFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    userfriendlist[userid][friendid].SetStatus("Block");

    if (end)
        result = "Block successful!";
    else  
        result = "Block failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::AddFriend(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int applicantid;  //发送好友申请
    int userid;  //接收好友申请
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", applicantid);

    userlist_[userid].AddApply(applicantid);

    if (end)
        result = "Add apply successful!";
    else  
        result = "Add apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> applyset = userlist_[userid].GetApplyList();
    std::unordered_map<int, bool> applylist;
    for (auto& it : applyset)
    {
        applylist[it] = userlist_[it].IsOnLine();
    }

    json reply_js = js_FriendList(userid, applylist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessFriendApply(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int applicantid;
    bool real_result;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "applicantid", applicantid);
    end &= AssignIfPresent(js, "result", real_result);

    userlist_[userid].DeleteApply(applicantid);
    if (real_result)
    {
        userlist_[userid].AddFriend(applicantid);
        userfriendlist[userid][applicantid] = std::move(Friend(userid, applicantid));
        userlist_[applicantid].AddFriend(userid);
        userfriendlist[applicantid][userid] = std::move(Friend(applicantid,userid));
    }

    if (end)
        result = "Process apply successful!";
    else  
        result = "Process apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> groupset = userlist_[userid].GetGroupList();
    std::vector<int> grouplist;

    for (auto&it : groupset)
    {
        grouplist.push_back(it);
    }

    json reply_js = js_UserList(grouplist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::CreateGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    std::string groupname;
    int creatorid;
    std::vector<int> othermembers;
    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "groupname", groupname);
    end &= AssignIfPresent(js, "creatorid", creatorid);
    end &= AssignIfPresent(js, "othermember", othermembers);

    grouplist_[groupid] = {groupid, groupname};
    grouplist_[groupid].AddMember(std::move(GroupUser(creatorid, "Group_owner")));
    for (auto& it : othermembers)
    {
        grouplist_[groupid].AddMember(std::move(GroupUser(it, "Member")));
    }

    userlist_[creatorid].JoinGroup(groupid);
    for (auto& it : othermembers)
    {
        userlist_[it].JoinGroup(groupid);
    }
    
    if (end)
        result = "Create group successful!";
    else  
        result = "Create group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupMemberList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int groupid;
    end &= AssignIfPresent(js, "groupid", groupid);
    
    std::vector<int> memberlist;
    if (end) 
    {
        std::unordered_map<int, GroupUser> temp = grouplist_[groupid].GetAllMembers();
        for (auto& it : temp) 
        {
            memberlist.push_back(it.second.GetUserId());
        }
    }

    json reply_js = js_UserList(memberlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListGroupApplyList(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int groupid;
    end &= AssignIfPresent(js, "groupid", groupid);

    std::vector<int> applylist_;
    if (end)
    {
        for (auto& it : grouplist_[groupid].GetApplyList())
        {
            applylist_.push_back(it);
        }
    }
    
    json reply_js = js_UserList(applylist_);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessGroupApply(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    int applicantid;
    bool real_result;
    end &= AssignIfPresent(js, "userid", groupid);
    end &= AssignIfPresent(js, "applicantid", applicantid);
    end &= AssignIfPresent(js, "result", real_result);

    grouplist_[groupid].DeleteApply(applicantid);
    if (real_result)
    {
        grouplist_[groupid].AddMember(std::move(GroupUser(applicantid, "Member")));
    }

    if (end)
        result = "Process apply successful!";
    else  
        result = "Process apply failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::QuitGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    userlist_[userid].LeaveGroup(groupid);
    grouplist_[groupid].RemoveMember(userid);

    if (end)
        result = "Quit group successful!";
    else  
        result = "Quit group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteGroup(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (grouplist_[groupid].GetMember(userid)->GetRole() == "Group_owner")
    {
        for (auto& it : grouplist_[groupid].GetAllMembers())
        {
            userlist_[it.first].LeaveGroup(groupid);
        }

        grouplist_.erase(groupid);
    }
    else 
    {
        end = false;
    }

    if (end)
        result = "Delete group successful!";
    else  
        result = "Delete group failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::PrintUserData(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    
    int userid;
    end &= AssignIfPresent(js, "userid", userid);
    
    std::string username = userlist_[userid].GetUserName();
    int friendnum = userlist_[userid].GetFriendList().size();
    int groupnum = userlist_[userid].GetGroupList().size();

    json reply_js = js_UserData(userid, username, friendnum, groupnum);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ChangeUserPassword(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    userlist_[userid].SetPassWord(password);

    if (end)
        result = "Chance password successful!";
    else  
        result = "Chance password failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteUserAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (userlist_[userid].GetPassWord() == password)
    {
        for (auto& it : userlist_[userid].GetFriendList())
        {
            userlist_[it.first].DeleteFriend(userid);
        }
        
        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist_[it].RemoveMember(userid);
        }

        for (auto& it : userfriendlist[userid])
        {
            userlist_[it.second.GetFriendId()].DeleteFriend(userid);
            userfriendlist[it.second.GetFriendId()].erase(userid);
        }
        
        userfriendlist.erase(userid);
        userlist_.erase(userid);
    }
    else {
        end = false;
    }

    if (end)
        result = "Delete account successful!";
    else  
        result = "Delete account failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}