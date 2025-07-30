cmake_minimum_required(VERSION 3.14)
project(mynet)

# 开启 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出静态库
add_library(mynet STATIC)

# 添加子目录
add_subdirectory(base)
add_subdirectory(logging)
add_subdirectory(net)
add_subdirectory(timer)

# 最后设置输出目录（可选）
set_target_properties(mynet PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
