#include "../client/Client.h"

int main ()
{
    EventLoop loop;

    Client client(loop, 10101, "127.0.0.1", "ChatClient");

    client.start();

    std::thread th([&client]() {
        client.InputLoop();
    });

    loop.loop();
    
    if (th.joinable())
        th.join();

    return 0;
}