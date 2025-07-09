cmake_minimum_required(VERSION 3.14)
project(ChatSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/database
    ${PROJECT_SOURCE_DIR}/manager
    ${PROJECT_SOURCE_DIR}/model
    ${PROJECT_SOURCE_DIR}/muduo
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/logging
    ${PROJECT_SOURCE_DIR}/muduo/net
    ${PROJECT_SOURCE_DIR}/muduo/net/poller
    ${PROJECT_SOURCE_DIR}/muduo/net/tcp
    ${PROJECT_SOURCE_DIR}/muduo/timer
    ${PROJECT_SOURCE_DIR}/tool
    ${PROJECT_SOURCE_DIR}/server
    include_directories(/usr/include/sw)  # Redis++ headers
)

# 设置子模块（muduo）
add_subdirectory(muduo)

# 源码子目录
file(GLOB_RECURSE DATABASE_SRC database/*.cpp)
file(GLOB_RECURSE MANAGER_SRC manager/*.cpp)
file(GLOB_RECURSE SERVER_SRC server/*.cpp)
file(GLOB_RECURSE TOOL_SRC tool/*.cpp)
file(GLOB_RECURSE MAIN_SRC main/*.cpp)

# 生成库（除 main/client 以外的部分）
add_library(chatcore STATIC
    ${DATABASE_SRC}
    ${MANAGER_SRC}
    ${SERVER_SRC}
    ${TOOL_SRC}
)

target_link_libraries(chatcore
    mynet  # muduo库
    pthread
    mysqlclient
    hiredis
    redis++ 
)

# 可执行文件：server
add_executable(chat_server main/server.cpp)
target_link_libraries(chat_server chatcore)

# 可执行文件：client（如果需要）
add_executable(chat_client main/client.cpp)
target_link_libraries(chat_client chatcore)
