# base模块

# 收集所有cpp源文件
set(BASE_SRC
    CurrentThread.cpp
    Thread.cpp
    Timestamp.cpp
)

# 添加到主库
target_sources(mynet PRIVATE ${BASE_SRC})

# 添加头文件路径
target_include_directories(mynet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
