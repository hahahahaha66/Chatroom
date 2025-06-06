#include "Clientservice.h"
#include "Client.h"

void Clientservice::LoginRequest(const TcpConnectionPtr& conn, const std::string& username, const std::string& password, const uint16_t seq)
{
    json packet  = js_Login(username, password);
    conn->send(codec_.encode(packet, 1, seq));
}
