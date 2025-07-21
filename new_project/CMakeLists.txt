cmake_minimum_required(VERSION 3.10)
project(ChatSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 include 目录
include_directories(
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/database
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/tool
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/net
    /usr/include/mysql               # MySQL 头文件路径
    /usr/local/include              # sw::redis++ 头文件路径
)

# 添加 muduo 链接库路径
link_directories(${PROJECT_SOURCE_DIR}/muduo/build/lib)

# 服务端专用源码（包含数据库、管理、工具模块）
file(GLOB_RECURSE SERVER_COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/database/*.cpp
    ${PROJECT_SOURCE_DIR}/tool/*.cpp
)

# 客户端专用源码（只包含工具模块，不含数据库和管理模块）
file(GLOB_RECURSE CLIENT_COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/tool/*.cpp
)

# ========= 服务端 =========
add_executable(server
    ${PROJECT_SOURCE_DIR}/main/main_server.cpp
    ${PROJECT_SOURCE_DIR}/server/Server.cpp
    ${PROJECT_SOURCE_DIR}/server/Service.cpp
    ${SERVER_COMMON_SOURCES}
)

target_link_libraries(server
    mynet
    pthread
    mysqlclient
    hiredis
    redis++                         # sw::redis++
)

# ========= 客户端 =========
add_executable(client
    ${PROJECT_SOURCE_DIR}/main/main_client.cpp
    ${PROJECT_SOURCE_DIR}/client/Client.cpp
    ${CLIENT_COMMON_SOURCES}
)

target_link_libraries(client
    mynet
    pthread
)
