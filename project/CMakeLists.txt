cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# 查找 MariaDB 库路径 (Arch Linux 专用)
find_path(MYSQL_INCLUDE_DIR 
    NAMES mysql/mysql.h
    PATHS /usr/include/mariadb /usr/include)
    
find_library(MYSQL_LIBRARY 
    NAMES mariadb libmariadb.so libmysqlclient.so 
    PATHS /usr/lib /usr/lib64)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MariaDB libraries not found")
endif()

# 头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/database
    ${PROJECT_SOURCE_DIR}/entity
    ${PROJECT_SOURCE_DIR}/muduo/base
    ${PROJECT_SOURCE_DIR}/muduo/net
    ${PROJECT_SOURCE_DIR}/muduo/logging
    ${PROJECT_SOURCE_DIR}/tool
    ${MYSQL_INCLUDE_DIR}
)

# 添加 muduo 静态库
link_directories(${PROJECT_SOURCE_DIR}/muduo/build/lib)

# 公共源码文件（被客户端和服务器共享）
set(COMMON_SRC
    database/DatabaseThreadPool.cpp
    database/MysqlConnection.cpp
    database/MysqlConnectionpool.cpp
    entity/Friend.cpp
    entity/Group.cpp
    entity/GroupUser.cpp
    entity/User.cpp
    json_protocol.cpp
    tool/Codec.cpp
)

# 客户端特定文件
set(CLIENT_SRC
    client/Client.cpp
    client/Clientservice.cpp
    main_Client.cpp
)

# 服务器特定文件
set(SERVER_SRC
    server/Server.cpp
    server/Service.cpp
    main_Server.cpp
)

# 客户端可执行文件
add_executable(chat_client 
    ${CLIENT_SRC}
    ${COMMON_SRC}
)
target_link_libraries(chat_client 
    mynet 
    ${MYSQL_LIBRARY}
    pthread
    crypto
    z
)

# 服务器可执行文件
add_executable(chat_server 
    ${SERVER_SRC}
    ${COMMON_SRC}
)
target_link_libraries(chat_server 
    mynet 
    ${MYSQL_LIBRARY}
    pthread
    crypto
    z
)

# 安装可执行文件
install(TARGETS chat_client chat_server
    RUNTIME DESTINATION bin
)