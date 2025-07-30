# net模块

# 收集net下直接cpp文件
set(NET_SRC
    Buffer.cpp
    Channel.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
)

# poller子模块
set(NET_POLLER_SRC
    poller/DefaultPoller.cpp
    poller/EPollPoller.cpp
    poller/Poller.cpp
)

# tcp子模块
set(NET_TCP_SRC
    tcp/Acceptor.cpp
    tcp/InetAddress.cpp
    tcp/Socket.cpp
    tcp/TcpConnection.cpp
    tcp/TcpServer.cpp
    tcp/Connector.cpp
    tcp/TcpClient.cpp
    tcp/Callback.cpp
)

# 汇总
set(NET_ALL_SRC
    ${NET_SRC}
    ${NET_POLLER_SRC}
    ${NET_TCP_SRC}
)

target_sources(mynet PRIVATE ${NET_ALL_SRC})
target_include_directories(mynet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/poller
    ${CMAKE_CURRENT_SOURCE_DIR}/tcp
)
