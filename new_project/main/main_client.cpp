#include "../client/Client.h"

int main ()
{
    EventLoop loop;
    InetAddress addr(10101, "10.30.0.120");

    Client client(loop, addr, "ChatClient");

    client.start();

    std::thread th([&client]() {
        client.InputLoop();
    });

    loop.loop();
    
    if (th.joinable())
        th.join();

    return 0;
}