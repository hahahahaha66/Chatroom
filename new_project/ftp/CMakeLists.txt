cmake_minimum_required(VERSION 3.10)
project(ftp_module)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_BUILD_TYPE Debug)

# 添加头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../tool
    ${PROJECT_SOURCE_DIR}/../muduo/base
    ${PROJECT_SOURCE_DIR}/../muduo/logging
    ${PROJECT_SOURCE_DIR}/../muduo/net
    ${PROJECT_SOURCE_DIR}/../muduo/net/tcp
    ${PROJECT_SOURCE_DIR}/../muduo/net/poller
    ${PROJECT_SOURCE_DIR}/../muduo/timer
)

# 添加 muduo 静态库
link_directories(${PROJECT_SOURCE_DIR}/../muduo/build/lib)

# 添加源文件
set(FTP_SRCS
    FileClient.cpp
    FileServer.cpp
    ../tool/Codec.cpp
)

# testclient 可执行文件
add_executable(testclient testclient.cpp ${FTP_SRCS})

# testserver 可执行文件
add_executable(testserver testserver.cpp ${FTP_SRCS})

# 链接 muduo 静态库
target_link_libraries(testclient mynet pthread)
target_link_libraries(testserver mynet pthread)
