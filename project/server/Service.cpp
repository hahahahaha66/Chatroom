#include "Service.h"
#include "../json_protocol.hpp"
#include "../entity/other.h"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <locale>
#include <memory>
#include <mysql/mariadb_com.h>
#include <mysql/mysql.h>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

Service::Service(Dispatcher& dispatcher) : dispatcher_(dispatcher)
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
    handermap_[2] = std::bind(&Service::RegisterAccount, this, _1, _2, _3, _4);
    handermap_[3] = std::bind(&Service::ListFriendlist, this, _1, _2, _3, _4);
    handermap_[4] = std::bind(&Service::DeleteFriend, this, _1, _2, _3, _4);
    handermap_[5] = std::bind(&Service::BlockFriend, this, _1, _2, _3, _4);
    handermap_[6] = std::bind(&Service::AddFriend, this, _1, _2, _3, _4);
    handermap_[7] = std::bind(&Service::ListFriendApplyList, this, _1, _2, _3, _4);
    handermap_[8] = std::bind(&Service::ListGroupList, this, _1, _2, _3, _4);
    handermap_[9] = std::bind(&Service::CreateGroup, this, _1, _2, _3, _4);
    handermap_[10] = std::bind(&Service::ListGroupMemberList, this, _1, _2, _3, _4);
    handermap_[11] = std::bind(&Service::ListGroupApplyList, this, _1, _2, _3, _4);
    handermap_[12] = std::bind(&Service::ProcessFriendApply, this, _1, _2, _3, _4);
    handermap_[13] = std::bind(&Service::ProcessGroupApply, this, _1, _2, _3, _4);
    handermap_[14] = std::bind(&Service::QuitGroup, this, _1, _2, _3, _4);
    handermap_[15] = std::bind(&Service::PrintUserData, this, _1, _2, _3, _4);
    handermap_[16] = std::bind(&Service::ChangeUserPassword, this, _1, _2, _3, _4);
    handermap_[17] = std::bind(&Service::DeleteUserAccount, this, _1, _2, _3, _4);
    handermap_[18] = std::bind(&Service::DeleteGroup, this, _1, _2, _3, _4);
    handermap_[19] = std::bind(&Service::ProcessMessage, this, _1, _2, _3, _4);
    handermap_[20] = std::bind(&Service::GetUserChatInterface, this, _1, _2, _3, _4);
    handermap_[21] = std::bind(&Service::GetGroupChatInterface, this, _1, _2, _3, _4);
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

void Service::StartAutoFlushToDataBase(int seconds)
{
    running_ = true;
    flush_thread_ = std::thread([this, seconds]() {
        while (running_)
        {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
            FlushToDataBase();
        }
    });
}

void Service::StopAutoFlush()
{
    if (flush_thread_.joinable())
        flush_thread_.join();
}

void Service::FlushToDataBase()
{
    databasethreadpool_.EnqueueTask([this](MysqlConnection& conn) {
        for (auto& [userid, user] : userlist_)
        {
            std::string sql = FormatUpdateUser(user);
            conn.ExcuteUpdata(sql);

            for (auto& applyid : user.GetApplyList())
            {
                std::string sql = FormatUpdateUserApply(userid, applyid);
                conn.ExcuteUpdata(sql);
            }

            for (auto& [firendid, userfriend] : userfriendlist[userid])
            {
                std::string sql = FormatUpdateFriend(userfriend);
                conn.ExcuteUpdata(sql);
            }
        }

        for (auto& [groupid, group] : grouplist_)
        {
            std::string sql_group = FormatUpdateGroup(group);
            conn.ExcuteUpdata(sql_group);

            for (auto& applyid : group.GetApplyList())
            {
                std::string sql = FormatUqdateGroupApply(groupid, applyid);
                conn.ExcuteUpdata(sql);
            }

            for (auto& [userid, groupuser] : group.GetAllMembers())
            {
                std::string sql_groupuser = FormatUpdateGroupUser(groupid, groupuser);
                conn.ExcuteUpdata(sql_groupuser);
            }
        }
    });
}

std::string Service::Escape(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '\'' || c == '\"' || c == '\\') output += '\\';
        output += c;
    }
    return output;
}

std::string Service::FormatUpdateUser(const User& user)
{
    std::ostringstream oss;
    oss << "replace into user (id, name, password, online) values (" 
        << user.GetId() << ", "<< Escape(user.GetUserName()) << ", "
        << Escape(user.GetPassWord()) << ", " << (user.IsOnLine() ? 1 : 0) << ") ;";
    return oss.str();
}

std::string Service::FormatUpdateFriend(const Friend& userfriend)
{
    std::ostringstream oss;
    oss << "replace into friend (userid, friendid, status) values ("
        << userfriend.GetUserId() << ", " << userfriend.GetFriendId() << ", "
        << userfriend.GetStatus() << ") ;";
    return oss.str();
}

std::string Service::FormatUpdateGroup(const Group& group)
{
    std::ostringstream oss;
    oss << "replace into groups (id, groupname) values ("
        << group.GetGroupId() << ", " << Escape(group.GetGroupName())
        << ") ;";
    return oss.str();
}

std::string Service::FormatUpdateGroupUser(const int& groupid, const GroupUser& groupuser)
{
    std::ostringstream oss;
    oss << "replace into groupuser (id, groupid, level) values ("
        << groupuser.GetUserId() << "," << groupid << ", "
        << Escape(groupuser.GetRole()) << ") ;";
    return oss.str();
}

std::string Service::FormatUpdateUserApply(const int& userid, const int& applyid)
{
    std::ostringstream oss;
    oss << "replace into userapply (userid, applyid) values ("
        << userid << applyid << ") ;";
    return oss.str();
}

std::string Service::FormatUqdateGroupApply(const int& groupid, const int& applyid)
{
    std::ostringstream oss;
    oss << "replace into groupapply (groupid, applyid) values ("
        << groupid << applyid << ") ;";
    return oss.str();
}

void Service::ProcessMessage(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
{
    bool end = true;

    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    std::string status;
    end &= AssignIfPresent(js, "type", type);
    end &= AssignIfPresent(js, "senderid", senderid);
    end &= AssignIfPresent(js, "receiverid", receiverid);
    end &= AssignIfPresent(js, "connect", connect);
    end &= AssignIfPresent(js, "status", status);

    if (end)  
    {
        if (type == "private")
        {
            if (chatconnect_[receiverid].type_ == ChatConnect::Type::Private && chatconnect_[receiverid].peerid_ == senderid)
            {
                std::shared_ptr<TcpConnection> receiver_conn = userlist_[receiverid].GetConnection();
                status = "read";
                json reply_js = js_Message(type, senderid, receiverid, connect, status);
                uint16_t type = (end == true ? 1 : 0);
                seq = 0;
                receiver_conn->send(codec_.encode(reply_js, type, seq));
            }
        }        
        else if (type == "group")
        {
            json reply_js = js_Message(type, senderid, receiverid, connect, status);
            for (auto& it : grouplist_[receiverid].GetAllMembers())
            {
                if (chatconnect_[it.first].type_ == ChatConnect::Type::Group && chatconnect_[it.first].peerid_ == receiverid)
                {
                    std::shared_ptr<TcpConnection> receiver_conn = userlist_[it.first].GetConnection();
                    uint16_t type = (end == true ? 1 : 0);
                    seq = 0;
                    receiver_conn->send(codec_.encode(reply_js, type, seq));
                }
            }
        }        
        else  
        {
            end = false;
        }    
    }

    if (end)
    {
        databasethreadpool_.EnqueueTask([type, senderid, receiverid, connect, status, this](MysqlConnection& conn) {
            std::ostringstream sql;
            sql << "insert into message (type, senderid, receiverid, connect, status) values ("
                << Escape(type) << ", " << senderid << ", " << receiverid << ", " << Escape(connect) << ", "
                << Escape(status) << ") ;";
            conn.ExcuteUpdata(sql.str());
        });
    }

}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
{
    bool end = true;
    std::unordered_map<int, std::string> friendlist;
    std::unordered_map<int, std::string> grouplist;

    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    int userid = -1;
    for (auto& it : userlist_)
    {
        if (it.second.GetUserName() == username)
        {
            userid = it.first;
            break;
        }
    }

    if (userid == -1) end = false;

    if (end)
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


void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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
            if (!res) 
            {
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

void Service::ListFriendlist(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::GetUserChatInterface(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
{
    bool end = true;
    
    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    std::promise<std::vector<Message>> promise;
    std::future<std::vector<Message>> result_future = promise.get_future();
    
    databasethreadpool_.EnqueueTask([userid, friendid, end, p = std::move(promise), this](MysqlConnection& conn)mutable {
        std::vector<Message> messages;

        std::ostringstream sql;
        sql << "select * from message where type = Private and ( ( senderid = " << userid 
            << " and receiverid = " << friendid << " ) " << "or ( senderid = " << friendid
            << " and receiverid = " << userid << " ) ) order by timestamp asc ;" ;
        MYSQL_RES* res = conn.ExcuteQuery(sql.str());
        if (!res)
        {
            p.set_value({});
            end = false;
            return;
        }

        MysqlResult result(res);
        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            std::string type = row.GetString("type");
            int senderid = row.GetInt("senderid");
            int receiverid = row.GetInt("receiverid");
            std::string connect = row.GetString("connect");
            std::string status = row.GetString("status");

            Message message_;
            message_.senderid_ = senderid;
            message_.receiverid_ = receiverid;
            message_.connnect_ = connect;
            message_.type_ = Message::Type::Private;

            if (status == "unread")
                message_.status_ = Message::Status::unread;
            else  
                message_.status_ = Message::Status::read;

            messages.push_back(std::move(message_));
        }

        return p.set_value(std::move(messages));
    });

    std::vector<Message> messages = result_future.get();

    json result;
    result["message"] = json::array();
    std::string status;
    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    for (const auto& msg : messages)
    {
        if (msg.status_ == Message::Status::read)
            status = "read";
        else  
            status = "unread";
 
        type = "Private";
        senderid = msg.senderid_;
        receiverid = msg.receiverid_;
        connect = msg.connnect_;    

        json one = js_Message(type, senderid, receiverid, connect, status);
        result["message"].push_back(one);
    }

    uint16_t type_ = (end == true ? 1 : 0);

    conn->send(codec_.encode(result, type_, seq));
}

void Service::GetGroupChatInterface(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    std::promise<std::vector<Message>> promise;
    std::future<std::vector<Message>> result_future = promise.get_future();

    databasethreadpool_.EnqueueTask([userid, groupid, end, p = std::move(promise), this](MysqlConnection& conn)mutable {
        std::vector<Message> messages;

        std::ostringstream sql;
        sql << "select * from message where type = Group and receiverid = " << groupid
            << " order by timestamp asc ;";
        
        MYSQL_RES* res = conn.ExcuteQuery(sql.str());
        if (!res)
        {
            p.set_value({});
            end = false;
            return;
        }

        MysqlResult result(res);
        while (result.Next())
        {
            MysqlRow row = result.GetRow();
            std::string type = row.GetString("type");
            int senderid = row.GetInt("senderid");
            int receiverid = row.GetInt("receiverid");
            std::string connect = row.GetString("connect");
            std::string status = row.GetString("status");

            Message message_;
            message_.senderid_ = senderid;
            message_.receiverid_ = receiverid;
            message_.connnect_ = connect;
            message_.type_ = Message::Type::Group;

            if (status == "unread")
                message_.status_ = Message::Status::unread;
            else  
                message_.status_ = Message::Status::read;

            messages.push_back(std::move(message_));
        }
        
        return p.set_value(std::move(messages));
    });

    std::vector<Message> messages = result_future.get();

    json result;
    result["message"] = json::array();
    std::string status;
    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    for (const auto& msg : messages)
    {
        if (msg.status_ == Message::Status::read)
            status = "read";
        else  
            status = "unread";
 
        type = "Group";
        senderid = msg.senderid_;
        receiverid = msg.receiverid_;
        connect = msg.connnect_;    

        json one = js_Message(type, senderid, receiverid, connect, status);
        result["message"].push_back(one);
    }

    uint16_t type_ = (end == true ? 1 : 0);

    conn->send(codec_.encode(result, type_, seq));
}  

void Service::DeleteFriend(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::BlockFriend(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::AddFriend(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ListFriendApplyList(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ListGroupList(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::CreateGroup(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ListGroupMemberList(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ListGroupApplyList(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ProcessGroupApply(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::QuitGroup(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::DeleteGroup(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::PrintUserData(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::ChangeUserPassword(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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

void Service::DeleteUserAccount(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
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