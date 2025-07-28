#include "../client/Client.h"

int main ()
{
    EventLoop loop;

    Client client(loop, 10101, "10.30.0.120", "ChatClient");

    client.start();

    std::thread th([&client]() {
        client.InputLoop();
    });

    loop.loop();
    
    if (th.joinable())
        th.join();

    return 0;
}