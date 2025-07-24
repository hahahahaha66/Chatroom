cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)

# 生成 client 可执行文件
add_executable(client src/client.cpp src/thread_pool.cpp)

# 生成 server 可执行文件
add_executable(server src/server.cpp src/thread_pool.cpp)

# 如果 thread_pool 需要 pthread 线程库，可以添加：
find_package(Threads REQUIRED)
target_link_libraries(client PRIVATE Threads::Threads)
target_link_libraries(server PRIVATE Threads::Threads)
