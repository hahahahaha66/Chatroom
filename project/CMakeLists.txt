cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# 头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/database
    ${PROJECT_SOURCE_DIR}/entity
    ${PROJECT_SOURCE_DIR}/muduo/base
)

# 添加muduo静态库（你可以根据实际路径调整）
link_directories(${PROJECT_SOURCE_DIR}/muduo/build/lib)

# 源码文件收集
file(GLOB CLIENT_SRC client/*.cpp)
file(GLOB DB_SRC database/*.cpp)
file(GLOB ENTITY_SRC entity/*.cpp)
set(JSON_SRC json_protocol.cpp)

# 可执行文件（你可以根据实际改名为 server / client 等）
add_executable(chatroom
    ${CLIENT_SRC}
    ${DB_SRC}
    ${ENTITY_SRC}
    ${JSON_SRC}
)

# 链接muduo库（你可能需要改为多个库）
target_link_libraries(chatroom mynet mysqlclient)
