# Chatroom

# 终端聊天室系统

本项目是一个基于 [Muduo](https://github.com/chenshuo/muduo) 网络库开发的 **终端聊天室系统**，采用 C++ 实现，强调 **高性能与模块化架构**。

## 快速开始

### 依赖安装（以 Arch Linux 为例）

客户端使用

```bash
sudo pacman -S cmake gperftools jemalloc jsoncpp
yay -S nlohmann-json

```
服务器使用
```bash
sudo pacman -S mariadb hiredis openssl gperftools jemalloc curl
yay -S redis-plus-plus nlohmann-json
```

### 编译
```bash
# 先到/new_project/muduo下
mkdir build && cd build
cmake ..
make -j
```
客户端
```bash
# 到/new_project/main_client下
mkdir build && cd build
cmake ..
make -j
# 运行./client
```
服务器
```bash
# 到/new_project/main_server下
mkdir build && cd build
cmake ..
make -j
# 运行./server
```

## 功能特性

-  账户系统（注册 / 登录）
-  好友管理（添加 / 删除 / 好友列表）
-  单聊与群聊
-  离线消息 / 聊天记录持久化
-  文件传输（上传 / 下载）
-  服务端集成 **MySQL** 和 **Redis**
-  性能分析支持（gperftools）

## 架构设计

### 服务端模块

- **Dispatcher**：消息类型分发器
- **Codec**：基于 “包头 + JSON” 的编解码器
- **线程池 / 连接池**：高并发处理能力
- **缓存同步机制**：定期将 Redis 缓存写入 MySQL

### 客户端设计

- **Dispatcher**：消息类型分发器
- **Codec**：基于 “包头 + JSON” 的编解码器
- 使用双线程，一个处理输出，一个处理输入

### 文件传输设计

- **零拷贝**：使用sendfile减少CPU拷贝，提高传输效率

### 通信协议

- 每条消息 = `[包长度][消息类型长度][消息类型][JSON正文]`
- 消息体使用 [nlohmann/json](https://github.com/nlohmann/json) 编解码

## 技术栈

| 类型        | 使用技术         |
|-------------|------------------|
| 网络通信    | Muduo            |
| 数据存储    | MySQL / Redis    |
| JSON解析    | nlohmann/json    |
| 性能分析    | gperftools       |
| 构建工具    | CMake            |
