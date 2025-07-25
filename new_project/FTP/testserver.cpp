#include "FileServer.h"
#include <cassert>

int main ()
{
    EventLoop loop_;
    InetAddress addr(9090, "127.0.0.1");
    FileServer server_(&loop_, addr, "fileserver");

    server_.start();
    loop_.loop();
    return 0;
}