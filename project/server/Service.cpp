#include "Service.h"
#include "../json_protocol.hpp"
#include "../entity/other.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mysql/mariadb_com.h>
#include <mysql/mysql.h>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

Service::Service(Dispatcher& dispatcher) : dispatcher_(dispatcher)
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
    handermap_[2] = std::bind(&Service::RegisterAccount, this, _1, _2, _3, _4);
    handermap_[3] = std::bind(&Service::DeleteFriend, this, _1, _2, _3, _4);
    handermap_[4] = std::bind(&Service::BlockFriend, this, _1, _2, _3, _4);
    handermap_[6] = std::bind(&Service::AddFriend, this, _1, _2, _3, _4);
    handermap_[9] = std::bind(&Service::CreateGroup, this, _1, _2, _3, _4);
    handermap_[12] = std::bind(&Service::ProcessFriendApply, this, _1, _2, _3, _4);
    handermap_[13] = std::bind(&Service::ProcessGroupApply, this, _1, _2, _3, _4);
    handermap_[14] = std::bind(&Service::QuitGroup, this, _1, _2, _3, _4);
    handermap_[17] = std::bind(&Service::DeleteUserAccount, this, _1, _2, _3, _4);
    handermap_[18] = std::bind(&Service::DeleteGroup, this, _1, _2, _3, _4);
    handermap_[19] = std::bind(&Service::ProcessMessage, this, _1, _2, _3, _4);
    handermap_[20] = std::bind(&Service::GetUserChatInterface, this, _1, _2, _3, _4);
    handermap_[21] = std::bind(&Service::GetGroupChatInterface, this, _1, _2, _3, _4);

    LOG_INFO << "Reading data from the database for initialization";

    ReadUserFromDataBase();
    ReadGroupFromDataBase();
    ReadFriendFromDataBase();
    ReadChatConnectFromDataBase();
    ReadGroupApplyFromDataBase();
    ReadUserApplyFromDataBase();
    ReadGroupUserFromDataBase();
    ReadMessageFromDataBase();

    LOG_INFO << "Database reading completed";

    StartAutoFlushToDataBase();
}

Service::~Service()
{
    StopAutoFlush();
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

        userfriendlist[userid][friendid] = std::move(Friend(userid, friendid, status));
    });

    for (auto& it : userlist_)
    {
        for (auto& fd: userfriendlist[it.first])
        {
            fd.second.SetFriendName(userlist_[fd.second.GetFriendId()].GetUserName());
            if (fd.second.GetStatus() == "Block")
            {
                userfriendlist[fd.first][it.first].SetBlocked(true);
            }
        }
    }
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
        grouplist_[groupid].AddApplyId(applyid);
    });
    
    for (auto& it : grouplist_)
    {
        for (auto& apply : it.second.GetApplyList())
        {
            it.second.AddApply(apply.second.userid_, userlist_[apply.second.userid_].GetUserName());
        }
    }
}

void Service::ReadUserApplyFromDataBase()
{
    std::string query = "select * from userapply;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int userid = row.GetInt("userid");
        int applyid = row.GetInt("applyid");
        userlist_[userid].AddApplyId(applyid);
    });

    for (auto& it : userlist_)
    {
        for (auto& apply : it.second.GetApplyList())
        {
            it.second.AddApply(apply.second.userid_, userlist_[apply.second.userid_].GetUserName());
        }
    }
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

void Service::ReadMessageFromDataBase()
{
    std::string query = "select * from message order by timestamp asc;";
    ReadFromDataBase(query, [this](MysqlRow& row) {
        int messageid = row.GetInt("id");
        int senderid = row.GetInt("senderid");
        int receiverid = row.GetInt("receiverid");
        std::string temp_type = row.GetString("type");
        std::string temp_status = row.GetString("status");
        std::string message = row.GetString("connect");
        std::string time = row.GetString("timestamp");
        Message::Type type;
        Message::Status status;

        if (temp_status == "read")
        {
            status = Message::Status::read;
        }
        else  
        {
            status = Message::Status::unread;
        }

        if (temp_type == "Group")
        {
            type = Message::Type::Group;
            groupmessage_[receiverid].emplace_back(std::move(Message(senderid, receiverid, message, status, type, time)));
        }
        else  
        {
            type = Message::Type::Private;
            usermessage_[senderid][receiverid].emplace_back(std::move(Message(senderid, receiverid, message, status, type, time)));
        }

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

            for (auto& it : user.GetApplyList())
            {
                std::string sql = FormatUpdateUserApply(userid, it.second.userid_);
                conn.ExcuteUpdata(sql);
            }

            for (auto& [firendid, userfriend] : userfriendlist[userid])
            {
                std::string sql = FormatUpdateFriend(userfriend);
                conn.ExcuteUpdata(sql);
            }

            for (auto& [friendid, message] : usermessage_[userid])
            {

                for (auto& it : message)
                {
                    std::string sql = FormatUpdateMessage(it);
                    conn.ExcuteUpdata(sql);
                }
            }
        }

        for (auto& [groupid, group] : grouplist_)
        {
            std::string sql_group = FormatUpdateGroup(group);
            conn.ExcuteUpdata(sql_group);

            for (auto& it : group.GetApplyList())
            {
                std::string sql = FormatUqdateGroupApply(groupid, it.second.userid_);
                conn.ExcuteUpdata(sql);
            }

            for (auto& [userid, groupuser] : group.GetAllMembers())
            {
                std::string sql_groupuser = FormatUpdateGroupUser(groupid, groupuser);
                conn.ExcuteUpdata(sql_groupuser);
            }

            for (auto& message : groupmessage_[groupid])
            {
                std::string sql = FormatUpdateMessage(message);
                conn.ExcuteUpdata(sql);
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
    oss << "replace into user (id, username, password, online) values (" 
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

std::string Service::FormatUpdateMessage(const Message& message)
{
    std::ostringstream oss;
    oss << "replace into message (type, senderid, receiverid, connect, status, timestamp) values ("
        << message.GetType() << message.senderid_ << message.receiverid_ 
        << message.connect_ << message.GetStatus() << message.time_ << ") ;";
    return oss.str();
}

void Service::ProcessMessage(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
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

    std::string time_ = time.toFormattedString();

    if (end)  
    {
        if (type == "private")
        {
            if (userlist_[receiverid].IsOnLine() && chatconnect_[receiverid].type_ == ChatConnect::Type::Private && chatconnect_[receiverid].peerid_ == senderid)
            {
                std::shared_ptr<TcpConnection> receiver_conn = userlist_[receiverid].GetConnection();
                status = "read";
                json reply_js = js_Message(type, senderid, receiverid, connect, status, time_);
                int type = (end == true ? 1 : 0);
                seq = 0;
                receiver_conn->send(codec_.encode(reply_js, type, seq));

                usermessage_[senderid][receiverid].emplace_back(std::move(Message(senderid, receiverid, connect, Message::Status::read, Message::Type::Private, time_)));
                usermessage_[receiverid][senderid].emplace_back(std::move(Message(senderid, receiverid, connect, Message::Status::read, Message::Type::Private, time_)));
            }
            else  
            {
                usermessage_[senderid][receiverid].emplace_back(std::move(Message(senderid, receiverid, connect, Message::Status::read, Message::Type::Private, time_)));
                usermessage_[receiverid][senderid].emplace_back(std::move(Message(senderid, receiverid, connect, Message::Status::unread, Message::Type::Private, time_)));
            }
        }        
        else if (type == "group")
        {
            json reply_js = js_Message(type, senderid, receiverid, connect, status, time_);
            for (auto& it : grouplist_[receiverid].GetAllMembers())
            {
                if (chatconnect_[it.first].type_ == ChatConnect::Type::Group && chatconnect_[it.first].peerid_ == receiverid)
                {
                    std::shared_ptr<TcpConnection> receiver_conn = userlist_[it.first].GetConnection();
                    int type = (end == true ? 1 : 0);
                    seq = 0;
                    receiver_conn->send(codec_.encode(reply_js, type, seq));
                }

                groupmessage_[receiverid].emplace_back(std::move(Message(senderid, receiverid, connect, Message::Status::unread, Message::Type::Private, time_)));
            }
        }        
        else  
        {
            end = false;
        }    
    }

    if (end)
    {
        json other_reply_js = js_CommandReply(true, "Message sent successful");
        int type = (end == true ? 1 : 0);
        seq = 14;
        conn->send(codec_.encode(other_reply_js, type, seq));
    }
    else  
    {
        json other_reply_js = js_CommandReply(true, "Message sent failed");
        int type = (end == true ? 1 : 0);
        seq = 14;
        conn->send(codec_.encode(other_reply_js, type, seq));
    }
}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::unordered_map<int, Friend> friendlist;
    std::unordered_map<int, Group> grouplist;
    std::unordered_map<int, SimpUser> friendapplylist;

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
    
    int type = (end == true ? 1 : 0);
    if (end) 
        userlist_[userid].SetOnline(conn);

    if (end)
    {
        for (auto& it : userfriendlist[userid])
        {
            userfriendlist[it.second.GetFriendId()][userid].SetOnline(true);
        }
    }

    if (end)
    {
        for (auto& it : userfriendlist[userid])
        {
            friendlist[it.first] = it.second;
        }

        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist[it] = grouplist_[it];
        }
    }

    if (end)
    {
        friendapplylist = userlist_[userid].GetApplyList();
    }

    json all_friend = js_UserAllData(userid, friendlist, grouplist, friendapplylist);
    conn->send(codec_.encode(all_friend, type, seq));
}


void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    LOG_INFO << js.dump();

    bool end = true;
    std::string result;

    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    std::promise<int> promise;
    std::future<int> result_future = promise.get_future();

    LOG_INFO << username << "  " << password ;

    if (end)
    {
        //检查数据库中有无已创建的账号，有返回false，没有则写入返回true
        auto p_shared = std::make_shared<std::promise<int>>(std::move(promise));
        databasethreadpool_.EnqueueTask([username, password, p = p_shared](MysqlConnection& conn)mutable {

            LOG_DEBUG << "Start database task for: " << username;

            std::string check_sql = 
                "select exists (select 1 from user where username = '" + username + "' ) as user_exists;";
            MYSQL_RES* res = conn.ExcuteQuery(check_sql);
            if (!res) 
            {
                p->set_value(-2);
                return ;
            }

            LOG_DEBUG << "Check query completed";

            MysqlResult result(res);
            if (result.Next())
            {
                MysqlRow row = result.GetRow();
                bool exists = row.GetBool("user_exists");
                if (exists)
                {
                    p->set_value(-1);
                    return;
                }
            }
            
            std::string insert_sql = 
                "insert into user (username, password) values (" + username + " , " + password + ");";
            if (!conn.ExcuteQuery(insert_sql)) 
            {
                p->set_value(-3);
                return;
            }

            LOG_DEBUG << "Insert query completed";


            MYSQL_RES* idres = conn.ExcuteQuery("select last_insert_id() as id");
            if (!idres)
            {
                p->set_value(-4);
                return;
            }

            MysqlResult id_result(idres);
            if (id_result.Next())
            {
                MysqlRow row = id_result.GetRow();
                int id = row.GetInt("id");
                p->set_value(id);
            }
            else  
            {
                p->set_value(-5);
            }
        });
    }

    int userid = result_future.get();

    LOG_INFO << userid ;

    userlist_.emplace(userid, User(userid, username, password));
    
    if (userid < 0)
        end = false;

    if (end)
        result = "Register successful!";
    else  
        result = "Register failed!";

    LOG_INFO << "registeraccount " << username << "  " << password << "  " << end;
    LOG_INFO << "result : " << result;

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);
    if (end)
    {
        userlist_[userid] = {userid, username, password};
    }
       
    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::GetUserChatInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    
    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);
    
    std::vector<Message> messages = usermessage_[userid][friendid];

    json result;
    result["message"] = json::array();
    std::string status;
    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    std::string time_;
    for (const auto& msg : messages)
    {
        if (msg.status_ == Message::Status::read)
            status = "read";
        else  
            status = "unread";
 
        type = "Private";
        senderid = msg.senderid_;
        receiverid = msg.receiverid_;
        connect = msg.connect_;   
        time_ = msg.time_; 

        json one = js_Message(type, senderid, receiverid, connect, status, time_);
        result["message"].push_back(one);
    }

    int type_ = (end == true ? 1 : 0);

    conn->send(codec_.encode(result, type_, seq));
}

void Service::GetGroupChatInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;

    int userid;
    int groupid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    std::vector<Message> messages = groupmessage_[groupid];

    json result;
    result["message"] = json::array();
    std::string status;
    std::string type;
    int senderid;
    int receiverid;
    std::string connect;
    std::string time_;
    for (const auto& msg : messages)
    {
        if (msg.status_ == Message::Status::read)
            status = "read";
        else  
            status = "unread";
 
        type = "Group";
        senderid = msg.senderid_;
        receiverid = msg.receiverid_;
        connect = msg.connect_;    
        time_ = msg.time_;

        json one = js_Message(type, senderid, receiverid, connect, status, time_);
        result["message"].push_back(one);
    }

    int type_ = (end == true ? 1 : 0);

    conn->send(codec_.encode(result, type_, seq));
}  

void Service::DeleteFriend(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    end &= userfriendlist[userid].erase(friendid);
    end &= userfriendlist[friendid].erase(userid);

    if (end)
        result = "Delete successful!";
    else  
        result = "Delete failed!";

    json data = js_UserWithFriend(userid, friendid);

    std::string json_str = data.dump();
    json reply_js = js_CommandReplyWithData(json_str, end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::BlockFriend(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    if (userfriendlist[userid][friendid].GetStatus() == "Block")
    {
        userfriendlist[userid][friendid].SetStatus("Normal");
        userfriendlist[friendid][userid].SetBlocked(false);
    }
    else  
    {
        userfriendlist[userid][friendid].SetStatus("Block");
        userfriendlist[friendid][userid].SetBlocked(true);
    }

    if (end)
        result = "Block successful!";
    else  
        result = "Block failed!";

    json data = js_UserWithFriend(userid, friendid);

    std::string json_str = data.dump();
    json reply_js = js_CommandReplyWithData(json_str, end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::AddFriend(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int applicantid;  //发送好友申请
    int userid;  //接收好友申请
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", applicantid);

    userlist_[userid].AddApply(applicantid, userlist_[applicantid].GetUserName());

    if (end)
        result = "Add apply successful!";
    else  
        result = "Add apply failed!";

    json reply_js = js_CommandReply(end, result);     
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessFriendApply(const TcpConnectionPtr& conn, const json& js, const int seq, Timestamp time)
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
        userfriendlist[userid][applicantid] = std::move(Friend(userid, applicantid));
        userfriendlist[applicantid][userid] = std::move(Friend(applicantid,userid));
    }

    if (end)
        result = "Process apply successful!";
    else  
        result = "Process apply failed!";

    json data = js_ApplyResult(userid, applicantid, userlist_[applicantid].GetUserName(), real_result);

    std::string json_str = data.dump();
    json reply_js = js_CommandReplyWithData(json_str, end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::AddGroup(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    int userid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    auto applylist = grouplist_[groupid].GetApplyList();
    auto it = applylist.find(userid);
    if (it != applylist.end())
    {
        grouplist_[groupid].AddApply(userid, userlist_[userid].GetUserName());
    }
    else  
    {
        end =false;
    }

    if (end)
        result = "Add group apply successful!";
    else  
        result = "Add group apply failed!";

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::CreateGroup(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int groupid;
    std::string groupname;
    int creatorid;
    std::vector<int> othermembers;

    end &= AssignIfPresent(js, "groupname", groupname);
    end &= AssignIfPresent(js, "creatorid", creatorid);
    end &= AssignIfPresent(js, "othermember", othermembers);

    std::promise<int> promise;
    std::future<int> result_future = promise.get_future();

    if (end)
    {
        //检查数据库中有无已创建的账号，有返回false，没有则写入返回true
        auto p_shared = std::make_shared<std::promise<int>>(std::move(promise));

        databasethreadpool_.EnqueueTask([groupname, creatorid, othermembers, p = p_shared](MysqlConnection& conn)mutable {
            std::string insert_sql = "insert into groups groupname values " + groupname + " ;";
            if (!conn.ExcuteQuery(insert_sql)) 
            {
                p->set_value(-3);
                return;
            }

            MYSQL_RES* idres = conn.ExcuteQuery("select last_insert_id() as is");
            if (!idres)
            {
                p->set_value(-4);
                return;
            }

            int groupid;
            MysqlResult id_result(idres);
            if (id_result.Next())
            {
                MysqlRow row = id_result.GetRow();
                groupid = row.GetInt("id");
                p->set_value(groupid);
            }
            else  
            {
                p->set_value(-5);
            }

            insert_sql = "insert into groupuser (id, groupid, level) values (" + std::to_string(creatorid) + 
                         " , " +  std::to_string(groupid) + " , " + "Group_owner" + ") ;";

            for (auto& it : othermembers)
            {
                 insert_sql = "insert into groupuser (id, groupid, level) values (" + std::to_string(it) + 
                         " , " +  std::to_string(groupid) + " , " + "Member" + ") ;";
            }
        });
    }

    groupid = result_future.get();

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
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ProcessGroupApply(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
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
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::QuitGroup(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
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
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::RemoveGroupUser(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
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
        result = "Remove groupuser successful!";
    else  
        result = "Remove groupuser failed!";

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::SetAdministrator(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    grouplist_[groupid].GetMember(userid)->SetRole("Administrator");

    if (end)
        result = "Set administrator successful!";
    else  
        result = "Set administrator failed!";

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::RemoveAdministrator(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int groupid;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "groupid", groupid);

    if (grouplist_[groupid].GetMember(userid)->GetRole() == "Administrator")
    {
        grouplist_[groupid].GetMember(userid)->SetRole("Member");
    }
    else  
    {
        end = false;
    }

    if (end)
        result = "Remove administrator successful!";
    else  
        result = "Remove administrator failed!";

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteGroup(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
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
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteUserAccount(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (userlist_[userid].GetPassWord() == password)
    {
        
        for (auto& it : userlist_[userid].GetGroupList())
        {
            grouplist_[it].RemoveMember(userid);
        }

        for (auto& it : userfriendlist[userid])
        {
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
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::UpdatedUserInterface(const TcpConnectionPtr& conn, const json& js, int seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int peeid;
    std::string temp_type;
    end &= AssignIfPresent(js, "peeid", peeid);
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "type", temp_type);
    ChatConnect::Type type_;

    if (temp_type == "Private")
    {
        type_ = ChatConnect::Type::Private;
    }
    else if (temp_type == "Group")
    {
        type_ = ChatConnect::Type::Group;
    }
    else if (temp_type == "None")
    {
        type_ = ChatConnect::Type::None;
    }
    else 
    {
        end = false;
    }

    if (end)
    {
        chatconnect_[userid].peerid_ = peeid;
        chatconnect_[userid].type_ = type_;
    }
   
    if (end)
        result = "Updated user interface successful!";
    else  
        result = "Updated user interface failed!";

    json reply_js = js_CommandReply(end, result);
    int type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::RefreshFriendDelete(int userid, int friendid, Timestamp time)
{
    if (userlist_[friendid].IsOnLine())
    {
        TcpConnectionPtr conn = userlist_[friendid].GetConnection();
        json reply_js = js_UserWithFriend(userid, friendid);

        conn->send(codec_.encode(reply_js, 1, -1));
    }
}

void Service::RefreshFriendBlock(int userid, int friendid, Timestamp time)
{
    if (userlist_[friendid].IsOnLine())
    {
        TcpConnectionPtr conn = userlist_[friendid].GetConnection();
        json reply_js = js_UserWithFriend(userid, friendid);

        conn->send(codec_.encode(reply_js, 1, -2));
    }
}

void Service::RefreshFriendAddApply(int userid, int friendid, Timestamp time)
{
    if (userlist_[friendid].IsOnLine())
    {
        TcpConnectionPtr conn = userlist_[friendid].GetConnection();
        json reply_js = js_Apply(friendid, userid, userlist_[userid].GetUserName());

        conn->send(codec_.encode(reply_js, 1, -3));   
    }
}

void Service::RefreshGroupAddApply(int groupid, int userid, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_Apply(groupid, userid, userlist_[userid].GetUserName());

            conn->send(codec_.encode(reply_js, 1, -4));
        }
    }
}

void Service::RefreshGroupCreate(int groupid, Timestamp time)
{
    TcpConnectionPtr conn;
    std::string groupname = grouplist_[groupid].GetGroupName();
    std::unordered_map<int, GroupUser> othermember = grouplist_[groupid].GetAllMembers();
    
    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_RefrushGroupCreate(groupid, groupname, othermember);

            conn->send(codec_.encode(reply_js, 1, -5));
        }
    }
}

void Service::RefreshFriendApplyProcess(int userid, int friendid, bool result, Timestamp time)
{
    if (userlist_[friendid].IsOnLine())
    {
        TcpConnectionPtr conn = userlist_[friendid].GetConnection();

        json reply_js = js_ApplyResult(friendid, userid, userlist_[userid].GetUserName(), result);
        conn->send(codec_.encode(reply_js, 1, -6));
    }
}

void Service::RefreshGroupApplyProcess(int groupid, int userid, bool result, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_ApplyResult(groupid, userid, userlist_[userid].GetUserName(), result);
            conn->send(codec_.encode(reply_js, 1, -8));
        }
    }
}

void Service::RefreshGroupQuitUser(int groupid, int userid, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_GroupData(groupid, userid);

            conn->send(codec_.encode(reply_js, 1, -8));
        }
    }
}

void Service::RefreshGroupRemoveUser(int groupid, int userid, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_GroupData(groupid, userid);

            conn->send(codec_.encode(reply_js, 1, -9));
        }
    }
}

void Service::RefreshGroupAddAdministrator(int groupid, int userid, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_GroupData(groupid, userid);

            conn->send(codec_.encode(reply_js, 1, -10));
        }
    }
}

void Service::RefreshGroupRemoveAdministrator(int groupid, int userid, Timestamp time)
{
    TcpConnectionPtr conn;

    for (auto& it : grouplist_[groupid].GetAllMembers())
    {
        if (userlist_[it.first].IsOnLine())
        {
            conn = userlist_[it.first].GetConnection();
            json reply_js = js_GroupData(groupid, userid);

            conn->send(codec_.encode(reply_js, 1, -11));
        }
    }
}