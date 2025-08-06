// while (!exitflag_)
//     {
//         if (currentState_ == "init_menu")
//         {
//             int order = 0;
//             std::string input;
//             std::cout << "1. 登陆" << std::endl;
//             std::cout << "2. 注册" << std::endl;
//             std::cout << "3. 退出" << std::endl;
//             getline(std::cin, input);
//             if (!ReadNum(input, order)) continue;

//             if (order == 1)
//             {
//                 std::cout << "输入邮箱: ";
//                 getline(std::cin, email_);
//                 std::cout << "输入密码: ";
//                 getline(std::cin, password_);

//                 json js = {
//                     {"email", email_},
//                     {"password", password_}
//                 };

//                 waitingback_ = true;
//                 Login(js);

//                 waitInPutReady();

//             }
//             else if (order == 2)
//             {
//                 std::cout << "输入邮箱: ";
//                 getline(std::cin, email_);
//                 std::cout << "输入用户名: ";
//                 getline(std::cin, name_);
//                 std::cout << "输入密码: ";
//                 getline(std::cin, password_);

//                 json js = {
//                     {"username", name_},
//                     {"email", email_},
//                     {"password", password_}
//                 };

//                 waitingback_ = true;
//                 Register(js);

//                 waitInPutReady();
//             }
//             else if (order == 3)
//             {
//                 std::cout << "正在退出..." << std::endl;
//                 stop();
//                 break;
//             }
//             else
//             {
//                 std::cout << "输入错误" << std::endl;
//             }
//         }
//         else if (currentState_ == "main_menu")
//         {
//             bool newapply = false;
//             for (auto it : friendapplylist_)
//             {
//                 if (it.second.status_ == "Pending")
//                 {
//                     newapply = true;
//                 }
//             }

//             json js = {
//                 {"userid", userid_},
//                 {"interfaceid", -1},
//                 {"interface", "Other"}
//             };
//             waitingback_ = true;
//             ChanceInterFace(js);

//             waitInPutReady();

//             int order;
//             std::string input;
//             std::cout << "1. 添加好友          " << "6. 屏蔽好友" <<
//             std::endl; std::cout << "2. 查看好友列表      " << "7. 删除好友"
//             << std::endl; std::cout << "3. 查看好友申请列表"; if (newapply)
//                 PrintfRed('*');
//             std::cout << "  " << "8. 好友界面" << std::endl;
//             std::cout << "4. 处理好友申请      " << "9. 群聊" << std::endl;
//             std::cout << "5. 查看被申请好友列表" << "10.注销账户"<<
//             std::endl; std::cout << "11.退出" << std::endl; getline(std::cin,
//             input); if (!ReadNum(input, order)) continue;

//             if (order == 1)
//             {
//                 std::string email;
//                 std::cout << "输入要添加的好友email:";
//                 getline(std::cin, email);

//                 if (email == email_)
//                 {
//                     std::cout << "邮箱为用户邮箱" << std::endl;
//                     continue;
//                 }
//                 for (auto it : friendlist_)
//                 {
//                     if (it.second.email_ == email)
//                     {
//                         std::cout << "该用户已经为你的好友" << std::endl;
//                         continue;
//                     }
//                 }

//                 for (auto it : friendsendapplylist_)
//                 {
//                     if (it.second.email_ == email && it.second.status_ ==
//                     "Pending")
//                     {
//                         std::cout << "已发送的好友申请,待处理中" <<
//                         std::endl; continue;
//                     }
//                 }

//                 json js = {
//                     {"fromid", userid_},
//                     {"targetemail", email}
//                 };
//                 waitingback_ = true;
//                 SendApply(js);

//                 waitInPutReady();
//             }
//             else if (order == 2)
//             {
//                 json js = {
//                     {"userid", userid_}
//                 };
//                 waitingback_ = true;
//                 ListFriend(js);

//                 waitInPutReady();
//                 for (auto it : friendlist_)
//                 {
//                     std::cout << "id: "<< it.first << "名字: " <<
//                     it.second.name_
//                               << "在线: " << it.second.online_ << "屏蔽" <<
//                               it.second.block_ ;
//                     if (it.second.new_ == true)
//                         PrintfRed('*');
//                     std::cout << std::endl;
//                 }
//             }
//             else if (order == 3)
//             {
//                 json js = {
//                     {"targetid", userid_}
//                 };
//                 waitingback_ = true;
//                 ListAllApply(js);

//                 waitInPutReady();
//                 for (auto it : friendapplylist_)
//                 {
//                     std::cout << "id: " << it.first << " name: " <<
//                     it.second.name_ << " status: " << it.second.status_ <<
//                     std::endl;
//                 }
//             }
//             else if (order == 4)
//             {
//                 json js = {
//                     {"targetid", userid_}
//                 };
//                 waitingback_ = true;
//                 ListAllApply(js);

//                 waitInPutReady();
//                 if (friendapplylist_.size() == 0)
//                 {
//                     std::cout << "当前无待处理的好友申请" << std::endl;
//                     continue;
//                 }

//                 for (auto it : friendapplylist_)
//                 {
//                     if (it.second.status_ == "Pending")
//                     {
//                         std::cout << "id: " << it.first << " name: " <<
//                         it.second.name_ << std::endl;
//                     }
//                 }
//                 while (true)
//                 {
//                     int fromid;
//                     std::string result;
//                     std::string input;
//                     std::cout << "输入处理的申请用户id(输入0返回): ";
//                     getline(std::cin, input);
//                     if (!ReadNum(input, fromid)) break;

//                     if (fromid == 0) break;
//                     std::cout << "输入选择(Agree或Reject): ";
//                     getline(std::cin, result);
//                     if (!(result == "Agree" || result == "Reject"))
//                     {
//                         std::cout << "输入错误,请重新操作" << std::endl;
//                         continue;
//                     }

//                     json js = {
//                         {"fromid", fromid},
//                         {"targetid", userid_},
//                         {"status", result}
//                     };
//                     waitingback_ = true;

//                     ProceFriendApply(js);
//                     waitInPutReady();
//                 }

//             }
//             else if (order == 5)
//             {
//                 json js = {
//                     {"fromid", userid_}
//                 };
//                 waitingback_ = true;
//                 ListAllSendApply(js);

//                 waitInPutReady();
//                 for (auto it : friendsendapplylist_)
//                 {
//                         std::cout << "id: " << it.first << " name: " <<
//                         it.second.name_ << " status: " << it.second.status_
//                         << std::endl;
//                 }
//             }
//             else if (order == 6)
//             {
//                 int friendid;
//                 bool block;
//                 bool nowblock;
//                 std::cout << "输入要屏蔽的好友id:";
//                 getline(std::cin, input);
//                 ReadNum(input, friendid);

//                 if (friendlist_.find(friendid) == friendlist_.end())
//                 {
//                     std::cout << "未知好友id" << std::endl;
//                     break;
//                 }
//                 else
//                 {
//                     nowblock = friendlist_[friendid].block_;
//                 }

//                 std::cout << "当前好友状态: ";
//                 if (nowblock)
//                     std::cout << "屏蔽中" << std::endl;
//                 else
//                     std::cout << "未屏蔽" << std::endl;

//                 std::cout << "输入选择(true或false)(0退出):";
//                 getline(std::cin, input);
//                 if (input == "0")
//                 {
//                     continue;
//                 }
//                 else if (input == "true")
//                 {
//                     if (nowblock == true)
//                         continue;
//                     else
//                         block = true;
//                 }
//                 else if (input == "false")
//                 {
//                     if (nowblock == false)
//                         continue;
//                     else
//                         block = false;
//                 }
//                 else
//                 {
//                     std::cout << "错误输入" << std::endl;
//                     continue;
//                 }

//                 json js = {
//                     {"userid", userid_},
//                     {"friendid", friendid},
//                     {"block", block}
//                 };
//                 waitingback_ = true;
//                 BlockFriend(js);

//                 waitInPutReady();
//             }
//             else if (order == 7)
//             {
//                 int friendid;
//                 std::cout << "输入要删除的好友id:";
//                 getline(std::cin, input);
//                 if (!ReadNum(input, friendid)) continue;

//                 if (friendlist_.find(friendid) == friendlist_.end())
//                 {
//                     std::cout << "未知好友id" << std::endl;
//                     break;
//                 }

//                 json js = {
//                     {"userid", userid_},
//                     {"friendid", friendid}
//                 };
//                 waitingback_ = true;
//                 DeleteFriend(js);

//                 waitInPutReady();
//             }
//             else if (order == 8)
//             {
//                 currentState_ = "friendchat_menu";
//             }
//             else if (order == 9)
//             {
//                 ClearScreen();
//                 currentState_ = "group_menu";
//             }
//             else if (order == 10)
//             {
//                 std::cout << "请输入密码" << std::endl;
//                 getline(std::cin, input);
//                 if (input != password_)
//                 {
//                     std::cout << "密码输入错误" << std::endl;
//                     break;
//                 }

//                 json js = {
//                     {"userid", userid_}
//                 };
//                 waitingback_ = true;
//                 DeleteAccount(js);

//                 waitInPutReady();
//             }
//             else if (order == 11)
//             {
//                 std::cout << "正在退出..." << std::endl;
//                 stop();
//                 break;
//             }
//             else
//             {
//                 std::cout << "错误指令,请重试" << std::endl;
//             }
//         }
//         else if (currentState_ == "friendchat_menu")
//         {
//             int friendid;
//             std::string input;
//             std::cout << "输入要进入的好友界面id(输入0退出): ";
//             getline(std::cin, input);
//             if (!ReadNum(input, friendid)) continue;
//             if (friendid == 0) currentState_ = "main_menu";

//             if (friendlist_.find(friendid) == friendlist_.end())
//             {
//                 std::cout << "未知好友id,请重新输入" << std::endl;
//                 continue;
//             }
//             else
//             {
//                 if (friendlist_[friendid].block_ == true)
//                 {
//                     std::cout << "好友已被屏蔽" << std::endl;
//                     continue;
//                 }
//             }

//             ClearScreen();

//             while (true)
//             {
//                 int order;
//                 std::cout << "1. 聊天" << std::endl;
//                 std::cout << "2. 发送文件" << std::endl;
//                 std::cout << "3. 查看聊天文件" << std::endl;
//                 std::cout << "4. 退出" << std::endl;
//                 getline(std::cin, input);
//                 if (!ReadNum(input, order)) continue;

//                 if (order == 1)
//                 {
//                         json js = {
//                         {"userid", userid_},
//                         {"interfaceid", friendid},
//                         {"interface", "Private"}
//                     };
//                     waitingback_ = true;
//                     ChanceInterFace(js);

//                     waitInPutReady();

//                     js = {
//                         {"userid", userid_},
//                         {"friendid", friendid},
//                     };
//                     waitingback_ = true;
//                     GetChatHistory(js);

//                     waitInPutReady();

//                     while (true)
//                     {
//                         std::string message;
//                         std::cout << "输入(输入0返回): " ;
//                         getline(std::cin, message);
//                         if (message == "0")
//                         {
//                             json js = {
//                                 {"userid", userid_},
//                                 {"interfaceid", -1},
//                                 {"interface", "Other"}
//                             };
//                             waitingback_ = true;
//                             ChanceInterFace(js);

//                             waitInPutReady();

//                             ClearScreen();
//                             break;
//                         }
//                         json js = {
//                             {"senderid", userid_},
//                             {"receiverid", friendid},
//                             {"content", message},
//                             {"type", "Private"},
//                             {"status", "Unread"},
//                         };
//                         friendlist_[friendid].maxmsgtime_ =
//                         GetCurrentTimestamp(); friendlist_[friendid].new_ =
//                         false; waitingback_ = true; SendMessage(js);

//                         waitInPutReady();
//                     }
//                     ClearScreen();
//                 }
//                 else if (order == 2)
//                 {
//                     std::string filename;
//                     std::string filepath;
//                     std::cout << "输入上传的文件名: ";
//                     getline(std::cin, filename);
//                     std::cout << "输入上传文件的完整路径(不用包括文件名): ";
//                     getline(std::cin, filepath);
//                     std::string temp = filepath + '/' + filename;
//                     std::cout << temp << std::endl;
//                     int tempfd = ::open(temp.c_str(), O_RDONLY);
//                     if (tempfd < 0)
//                     {
//                         std::cout << "无法打开文件" << std::endl;
//                         continue;
//                     }
//                     else
//                     {
//                         ::close(tempfd);
//                     }
//                     UploadFile(filename, filepath, friendid, "Private");
//                 }
//                 else if (order == 3)
//                 {
//                     json js = {
//                         {"userid", userid_},
//                         {"friendid", friendid}
//                     };
//                     waitingback_ = true;
//                     ListFriendFile(js);

//                     waitInPutReady();
//                     for (int i = 0; i < filelist_.size(); i++)
//                     {
//                         std::string sendname =
//                         friendlist_[filelist_[i].senderid_].name_; std::cout
//                         << i+1 << " " << sendname << ": " <<
//                         filelist_[i].filename_ << std::endl;
//                     }

//                     int id;
//                     std::cout << "输入要下载的文件标号(退出输入0)";
//                     getline(std::cin, input);
//                     if (!ReadNum(input, id)) continue;
//                     if (id == 0) continue;
//                     if (id > filelist_.size())
//                     {
//                         std::cout << "错误输入" << std::endl;
//                         continue;
//                     }
//                     id--;
//                     std::string filename = filelist_[id].filename_;
//                     std::string timestamp = filelist_[id].timestamp_;
//                     std::string savepath;
//                     std::cout << "输入保存路径";
//                     getline(std::cin, savepath);
//                     if (!(std::filesystem::exists(savepath) &&
//                     std::filesystem::is_directory(savepath)))
//                     {
//                         std::cout << "不存在的文件夹";
//                         continue;
//                     }
//                     DownloadFile(filename, savepath, timestamp);
//                     std::vector<File> newfilelist;
//                     filelist_ = std::move(newfilelist);
//                 }
//                 else if (order == 4)
//                 {
//                     currentState_ = "main_menu";
//                     ClearScreen();
//                     break;
//                 }
//                 else
//                 {
//                     std::cout << "错误输入" << std::endl;
//                     continue;
//                 }
//             }
//         }
//         else if (currentState_ == "group_menu")
//         {
//             json js = {
//                 {"userid", userid_},
//                 {"interfaceid", -1},
//                 {"interface", "Other"}
//             };
//             waitingback_ = true;
//             ChanceInterFace(js);

//             waitInPutReady();

//             int order = 0;
//             std::string input;
//             std::cout << "1. 创建群聊" << std::endl;
//             std::cout << "2. 加入群聊" << std::endl;
//             std::cout << "3. 查看群聊列表" << std::endl;
//             std::cout << "4. 查看发送的群聊申请" << std::endl;
//             std::cout << "5. 进入群聊" << std::endl;
//             std::cout << "6. 返回上一界面" << std::endl;

//             getline(std::cin, input);
//             if (!ReadNum(input, order)) continue;

//             if (order == 1)
//             {
//                 std::string groupname;
//                 std::cout << "输入创建的群聊名称(输入0返回): ";
//                 getline(std::cin, groupname);
//                 if (groupname == "0") continue;

//                 json js = {
//                     {"groupname", groupname},
//                     {"ownerid", userid_}
//                 };

//                 waitingback_ = true;
//                 CreateGroup(js);

//                 waitInPutReady();
//             }
//             else if (order == 2)
//             {
//                 bool same = false;
//                 std::string groupname;
//                 std::cout << "输入要加入的群聊名称(输入0返回): ";
//                 getline(std::cin, groupname);
//                 if (groupname == "0") continue;

//                 for (auto it : grouplist_)
//                 {
//                     if (it.second.name_ == groupname)
//                     {
//                         std::cout << "你已在群聊" << std::endl;
//                         same = true;
//                         break;
//                     }
//                 }
//                 if (same) continue;

//                 json js = {
//                     {"groupname", groupname},
//                     {"userid", userid_}
//                 };

//                 waitingback_ = true;
//                 SendGroupApply(js);

//                 waitInPutReady();
//             }
//             else if (order == 3)
//             {
//                 json js = {
//                     {"userid", userid_}
//                 };

//                 waitingback_ = true;
//                 ListGroup(js);

//                 waitInPutReady();

//                 std::cout << "id  " << "  名字" << "        "  << std::endl;
//                 for (auto it : grouplist_)
//                 {
//                     std::cout << it.first << " " << it.second.name_  << " ";
//                     if (it.second.newapply_ || it.second.newmessage_)
//                         PrintfRed('*');
//                     std::cout << std::endl;
//                 }
//             }
//             else if (order == 4)
//             {
//                 json js = {
//                     {"userid", userid_}
//                 };

//                 waitingback_ = true;
//                 ListSendGroupApply(js);

//                 waitInPutReady();
//             }
//             else if (order == 5)
//             {
//                 currentState_ = "groupchat_menu";
//                 continue;;
//             }
//             else if (order == 6)
//             {
//                 currentState_ = "main_menu";
//                 ClearScreen();
//                 continue;;
//             }
//             else
//             {
//                 std::cout << "错误输入" << std::endl;
//                 continue;
//             }
//         }
//         else if (currentState_ == "groupchat_menu")
//         {
//             std::string input;
//             int order;
//             int groupid;
//             std::cout << "输入要进入的群聊id: ";
//             getline(std::cin, input);
//             if (!ReadNum(input, groupid)) continue;

//             if (grouplist_.find(groupid) == grouplist_.end())
//             {
//                 std::cout << "未知群聊id,请重新输入" << std::endl;
//                 currentState_ = "group_menu";
//                 continue;
//             }

//             ClearScreen();
//             while (true)
//             {
//                 json js = {
//                     {"userid", userid_},
//                     {"interfaceid", -1},
//                     {"interface", "Other"}
//                 };
//                 waitingback_ = true;
//                 ChanceInterFace(js);

//                 waitInPutReady();

//                 std::string userrole;
//                 auto& userlist = grouplist_[groupid].groupuserlist_;
//                 if (userlist.find(userid_) != userlist.end())
//                 {
//                     userrole = userlist[userid_].role_;
//                 }
//                 else
//                 {
//                     currentState_ = "group_menu";
//                     continue;
//                 }

//                 std::cout << "1. 聊天";
//                 if (grouplist_[groupid].newmessage_)
//                     PrintfRed('*');
//                 std::cout << std::endl;
//                 std::cout << "2. 查看群聊成员" << std::endl;
//                 std::cout << "3. 发送文件" << std::endl;
//                 std::cout << "4. 查看群聊文件" << std::endl;
//                 std::cout << "5. 退出群聊" << std::endl;
//                 if (userrole== "Administrator" || userrole == "Owner")
//                 {
//                     std::cout << "6. 查看群聊申请";
//                     if (grouplist_[groupid].newapply_)
//                         PrintfRed('*');
//                     std::cout << std::endl;
//                     std::cout << "7. 处理群聊申请" << std::endl;
//                     std::cout << "8. 处理成员禁言" << std::endl;
//                     std::cout << "9. 移除成员" << std::endl;
//                 }
//                 if (userrole == "Owner")
//                 {
//                     std::cout << "10. 更改群聊成员权限" << std::endl;
//                     std::cout << "11. 解散群聊" << std::endl;
//                 }
//                 std::cout << "12. 返回上一界面" << std::endl;

//                 getline(std::cin, input);
//                 if (!ReadNum(input, order)) continue;

//                 if (order == 1)
//                 {
//                      js = {
//                         {"userid", userid_},
//                         {"interfaceid", groupid},
//                         {"interface", "Group"}
//                     };
//                     waitingback_ = true;
//                     ChanceInterFace(js);

//                     waitInPutReady();

//                     json js = {
//                         {"userid", userid_},
//                         {"groupid", groupid}
//                     };

//                     waitingback_ = true;
//                     GetGroupHistory(js);

//                     waitInPutReady();

//                     while (true)
//                     {
//                         std::string message;
//                         std::cout << "输入(输入0返回): " ;
//                         getline(std::cin, message);
//                         if (message == "0")
//                         {
//                             ClearScreen();
//                             break;
//                         }

//                         if (userlist.find(userid_) == userlist.end())
//                         {
//                             currentState_ = "group_menu";
//                             break;
//                         }

//                         bool usermute = userlist[userid_].mute_;
//                         if (usermute == true)
//                         {
//                             std::cout << "你已被禁言,无法发送消息" <<
//                             std::endl; continue;
//                         }

//                         json js = {
//                             {"senderid", userid_},
//                             {"receiverid", groupid},
//                             {"content", message},
//                             {"type", "Group"},
//                             {"status", "Unread"},
//                         };
//                         grouplist_[groupid].maxmsgtime_ =
//                         GetCurrentTimestamp();
//                         grouplist_[groupid].newmessage_ = false;
//                         waitingback_ = true;
//                         SendMessage(js);

//                         waitInPutReady();
//                     }
//                 }
//                 else if (order == 2)
//                 {
//                     json js = {
//                         {"groupid", groupid}
//                     };

//                     waitingback_ = true;
//                     GroupMember(js);

//                     for (auto it : grouplist_[groupid].groupuserlist_)
//                     {
//                         std::cout << it.first << "  " << it.second.username_
//                         << "  " << it.second.role_ << std::endl;
//                     }

//                     waitInPutReady();
//                 }
//                 else if (order == 3)
//                 {
//                     std::string filename;
//                     std::string filepath;
//                     std::cout << "输入上传的文件名: ";
//                     getline(std::cin, filename);
//                     std::cout << "输入上传文件的完整路径(不用包括文件名): ";
//                     getline(std::cin, filepath);
//                     std::string temp = filepath + '/' + filename;
//                     std::cout << temp << std::endl;
//                     int tempfd = ::open(temp.c_str(), O_RDONLY);
//                     if (tempfd < 0)
//                     {
//                         std::cout << "无法打开文件" << std::endl;
//                         continue;
//                     }
//                     else
//                     {
//                         ::close(tempfd);
//                     }
//                     UploadFile(filename, filepath, groupid, "Group");
//                 }
//                 else if (order == 4)
//                 {
//                     json js = {
//                         {"groupid",groupid}
//                     };
//                     waitingback_ = true;
//                     ListGroupFile(js);

//                     waitInPutReady();
//                     for (int i = 0; i < filelist_.size(); i++)
//                     {
//                         std::cout << i+1 << " " << filelist_[i].senderid_ <<
//                         ": " << filelist_[i].filename_ << std::endl;
//                     }

//                     int id;
//                     std::cout << "输入要下载的文件标号(退出输入0)";
//                     getline(std::cin, input);
//                     if (!ReadNum(input, id)) continue;
//                     if (id == 0) continue;
//                     if (id > filelist_.size())
//                     {
//                         std::cout << "错误输入" << std::endl;
//                         continue;
//                     }
//                     std::string filename = filelist_[id].filename_;
//                     std::string timestamp = filelist_[id].timestamp_;
//                     std::string savepath;
//                     std::cout << "输入保存路径";
//                     getline(std::cin, savepath);
//                     if (!(std::filesystem::exists(savepath) &&
//                     std::filesystem::is_directory(savepath)))
//                     {
//                         std::cout << "不存在的文件夹";
//                         continue;
//                     }
//                     DownloadFile(filename, savepath, timestamp);
//                     std::vector<File> newfilelist;
//                     filelist_ = std::move(newfilelist);
//                 }
//                 else if (order == 5)
//                 {
//                     json js = {
//                         {"groupid", groupid},
//                         {"userid", userid_}
//                     };

//                     waitingback_ = true;
//                     QuitGroup(js);

//                     waitInPutReady();
//                     currentState_ = "group_menu";
//                     break;
//                 }
//                 else if (order == 6)
//                 {
//                     json js = {
//                         {"groupid", groupid}
//                     };

//                     waitingback_ = true;
//                     ListGroupApply(js);

//                     waitInPutReady();
//                 }
//                 else if (order == 7)
//                 {
//                     int applyid;
//                     std::string status;
//                     std::cout << "输入要处理的申请用户的id: ";
//                     getline(std::cin, input);
//                     if (!ReadNum(input, applyid)) continue;

//                     std::cout << "输入选择(Agree或Reject): ";
//                     getline(std::cin, status);
//                     if (!(status == "Agree" || status == "Reject"))
//                     {
//                         std::cout << "输入错误,请重新操作" << std::endl;
//                         continue;
//                     }

//                     json js = {
//                         {"groupid", groupid},
//                         {"applyid", applyid},
//                         {"status", status}
//                     };

//                     waitingback_ = true;
//                     ProceGroupApply(js);

//                     waitInPutReady();
//                 }
//                 else if (order == 8)
//                 {
//                     int userid;
//                     bool block;
//                     bool nowblock;
//                     std::cout << "输入要禁言的成员id:";

//                     getline(std::cin, input);
//                     ReadNum(input, userid);

//                     if (userid == userid_)
//                     {
//                         std::cout << "不能禁言自己" << std::endl;
//                         continue;
//                     }
//                     auto userlist = grouplist_[groupid].groupuserlist_;
//                     if (userlist.find(userid) == userlist.end())
//                     {
//                         std::cout << "错误成员id" << std::endl;
//                         continue;
//                     }
//                     std::string userrole = userlist[userid_].role_;
//                     if (userrole != "Owner")
//                     {
//                         if (userlist[userid].role_ != "Member")
//                         {
//                             std::cout << "权限不足" << std::endl;
//                             continue;
//                         }
//                     }

//                     json js = {
//                         {"groupid", groupid},
//                         {"userid", userid},
//                     };
//                     waitingback_ = true;
//                     BlockGroupUser(js);

//                     waitInPutReady();
//                 }
//                 else if (order == 9)
//                 {
//                     int userid;
//                     std::cout << "输入要移除的成员id" << std::endl;
//                     getline(std::cin, input);
//                     if (!ReadNum(input, userid)) continue;
//                     if (userid == userid_)
//                     {
//                         std::cout << "不能移除自己" << std::endl;
//                         continue;
//                     }
//                     auto userlist = grouplist_[groupid].groupuserlist_;
//                     if (userlist.find(userid) == userlist.end())
//                     {
//                         std::cout << "错误成员id" << std::endl;
//                         continue;
//                     }
//                     std::string userrole = userlist[userid_].role_;
//                     if (userrole != "Owner")
//                     {
//                         if (userlist[userid].role_ != "Member")
//                         {
//                             std::cout << "权限不足" << std::endl;
//                             continue;
//                         }
//                     }

//                     json js = {
//                         {"userid", userid},
//                         {"groupid", groupid}
//                     };

//                     waitingback_ = true;
//                     RemoveGroupUser(js);

//                     waitInPutReady();
//                 }
//                 else if (order == 10)
//                 {
//                     int userid;
//                     std::string role;
//                     std::cout << "输入要改变的成员id: " << std::endl;
//                     getline(std::cin, input);
//                     if (!ReadNum(input, userid)) continue;
//                     if (userid == userid_)
//                     {
//                         std::cout << "不能改变自己" << std::endl;
//                         continue;
//                     }
//                     auto userlist = grouplist_[groupid].groupuserlist_;
//                     if (userlist.find(userid) == userlist.end())
//                     {
//                         std::cout << "错误成员id" << std::endl;
//                         continue;
//                     }
//                     std::string userrole = userlist[userid_].role_;
//                     if (userrole != "Owner")
//                     {
//                         if (userlist[userid].role_ != "Member")
//                         {
//                             std::cout << "权限不足" << std::endl;
//                             continue;
//                         }
//                     }
//                     std::cout << "输入改变后的角色(Member或Administrator): "
//                     << std::endl; getline(std::cin, role); if (!(role ==
//                     "Member" || role == "Administrator"))
//                     {
//                         std::cout << "输入错误" << std::endl;
//                         continue;
//                     }

//                     json js = {
//                         {"groupid", groupid},
//                         {"userid", userid},
//                         {"role", role}
//                     };

//                     waitingback_ = true;
//                     ChangeUserRole(js);

//                     waitInPutReady();
//                 }
//                 else if (order == 11)
//                 {
//                     json js = {
//                         {"groupid", groupid}
//                     };
//                     waitingback_ = true;
//                     DeleteGroup(js);

//                     waitInPutReady();
//                     if (currentState_ == "group_menu")
//                         break;
//                 }
//                 else if (order == 12)
//                 {
//                     currentState_ = "group_menu";
//                     ClearScreen();
//                     break;
//                 }
//                 else
//                 {
//                     std::cout << "错误指令,请重试" << std::endl;
//                 }
//             }
//         }
//         else
//         {
//             LOG_ERROR << "!!!!Wrong!!!!";
//             return;
//         }
//     }
