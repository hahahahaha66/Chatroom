#include "Callback.h"
#include "../../base/Timestamp.h"
#include <iostream> // 如果需要输出日志或调试

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    // 你的默认连接回调逻辑
    std::cout << "Default connection callback" << std::endl;
}

void defaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buffer,
                            Timestamp receiveTime) {
    // 你的默认消息回调逻辑
    std::cout << "Default message callback" << std::endl;
}
