cmake_minimum_required(VERSION 3.10)
project(ChatSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

# 添加 include 目录
include_directories(
    ${PROJECT_SOURCE_DIR}/../client
    ${PROJECT_SOURCE_DIR}/../database
    ${PROJECT_SOURCE_DIR}/../model
    ${PROJECT_SOURCE_DIR}/../server
    ${PROJECT_SOURCE_DIR}/../tool
    ${PROJECT_SOURCE_DIR}/../ftp
    ${PROJECT_SOURCE_DIR}/../muduo/base
    ${PROJECT_SOURCE_DIR}/../muduo/net
    /usr/include/mysql               # MySQL 头文件路径
    /usr/local/include              # sw::redis++ 头文件路径
)

# 添加 muduo 链接库路径
link_directories(${PROJECT_SOURCE_DIR}/../muduo/build/lib)

# 客户端专用源码（只包含工具模块，不含数据库和管理模块）
file(GLOB_RECURSE CLIENT_COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/../tool/*.cpp
)

# ========= 客户端 =========
add_executable(client
    ${PROJECT_SOURCE_DIR}/main_client.cpp
    ${PROJECT_SOURCE_DIR}/../client/Client.cpp
    ${PROJECT_SOURCE_DIR}/../ftp/FileClient.cpp
    ${CLIENT_COMMON_SOURCES}
)

target_link_libraries(client
    mynet
    pthread
    profiler
    jemalloc
)