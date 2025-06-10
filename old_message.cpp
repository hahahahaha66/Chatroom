// if (end)
// {
//     databasethreadpool_.EnqueueTask([type, senderid, receiverid, connect, status, this](MysqlConnection& conn) {
//         std::ostringstream sql;
//         sql << "insert into message (type, senderid, receiverid, connect, status) values ("
//             << Escape(type) << ", " << senderid << ", " << receiverid << ", " << Escape(connect) << ", "
//             << Escape(status) << ") ;";
//         conn.ExcuteUpdata(sql.str());
//     });
// }


// void Service::GetUserChatInterface(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
// {
//     bool end = true;
    
//     int userid;
//     int friendid;

//     end &= AssignIfPresent(js, "userid", userid);
//     end &= AssignIfPresent(js, "friendid", friendid);

//     std::promise<std::vector<Message>> promise;
//     std::future<std::vector<Message>> result_future = promise.get_future();
    
//     databasethreadpool_.EnqueueTask([userid, friendid, end, p = std::move(promise), this](MysqlConnection& conn)mutable {
//         std::vector<Message> messages;

//         std::ostringstream sql;
//         sql << "select * from message where type = Private and ( ( senderid = " << userid 
//             << " and receiverid = " << friendid << " ) " << "or ( senderid = " << friendid
//             << " and receiverid = " << userid << " ) ) order by timestamp asc ;" ;
//         MYSQL_RES* res = conn.ExcuteQuery(sql.str());
//         if (!res)
//         {
//             p.set_value({});
//             end = false;
//             return;
//         }

//         MysqlResult result(res);
//         while (result.Next())
//         {
//             MysqlRow row = result.GetRow();
//             std::string type = row.GetString("type");
//             int senderid = row.GetInt("senderid");
//             int receiverid = row.GetInt("receiverid");
//             std::string connect = row.GetString("connect");
//             std::string status = row.GetString("status");

//             Message message_(senderid, receiverid, connect);
//             message_.type_ = Message::Type::Private;

//             if (status == "unread")
//                 message_.status_ = Message::Status::unread;
//             else  
//                 message_.status_ = Message::Status::read;

//             messages.push_back(std::move(message_));
//         }

//         return p.set_value(std::move(messages));
//     });

//     std::vector<Message> messages = result_future.get();

//     json result;
//     result["message"] = json::array();
//     std::string status;
//     std::string type;
//     int senderid;
//     int receiverid;
//     std::string connect;
//     for (const auto& msg : messages)
//     {
//         if (msg.status_ == Message::Status::read)
//             status = "read";
//         else  
//             status = "unread";
 
//         type = "Private";
//         senderid = msg.senderid_;
//         receiverid = msg.receiverid_;
//         connect = msg.connect_;    

//         json one = js_Message(type, senderid, receiverid, connect, status);
//         result["message"].push_back(one);
//     }

//     uint16_t type_ = (end == true ? 1 : 0);

//     conn->send(codec_.encode(result, type_, seq));
// }

// void Service::GetGroupChatInterface(const TcpConnectionPtr& conn, const json& js, uint16_t seq, Timestamp time)
// {
//     bool end = true;

//     int userid;
//     int groupid;

//     end &= AssignIfPresent(js, "userid", userid);
//     end &= AssignIfPresent(js, "groupid", groupid);

//     std::promise<std::vector<Message>> promise;
//     std::future<std::vector<Message>> result_future = promise.get_future();

//     databasethreadpool_.EnqueueTask([userid, groupid, end, p = std::move(promise), this](MysqlConnection& conn)mutable {
//         std::vector<Message> messages;

//         std::ostringstream sql;
//         sql << "select * from message where type = Group and receiverid = " << groupid
//             << " order by timestamp asc ;";
        
//         MYSQL_RES* res = conn.ExcuteQuery(sql.str());
//         if (!res)
//         {
//             p.set_value({});
//             end = false;
//             return;
//         }

//         MysqlResult result(res);
//         while (result.Next())
//         {
//             MysqlRow row = result.GetRow();
//             std::string type = row.GetString("type");
//             int senderid = row.GetInt("senderid");
//             int receiverid = row.GetInt("receiverid");
//             std::string connect = row.GetString("connect");
//             std::string status = row.GetString("status");

//             Message message_(senderid, receiverid, connect);
//             message_.type_ = Message::Type::Group;

//             if (status == "unread")
//                 message_.status_ = Message::Status::unread;
//             else  
//                 message_.status_ = Message::Status::read;

//             messages.push_back(std::move(message_));
//         }
        
//         return p.set_value(std::move(messages));
//     });

//     std::vector<Message> messages = result_future.get();

//     json result;
//     result["message"] = json::array();
//     std::string status;
//     std::string type;
//     int senderid;
//     int receiverid;
//     std::string connect;
//     for (const auto& msg : messages)
//     {
//         if (msg.status_ == Message::Status::read)
//             status = "read";
//         else  
//             status = "unread";
 
//         type = "Group";
//         senderid = msg.senderid_;
//         receiverid = msg.receiverid_;
//         connect = msg.connect_;    

//         json one = js_Message(type, senderid, receiverid, connect, status);
//         result["message"].push_back(one);
//     }

//     uint16_t type_ = (end == true ? 1 : 0);

//     conn->send(codec_.encode(result, type_, seq));
// }  