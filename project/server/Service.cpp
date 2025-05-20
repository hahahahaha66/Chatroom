#include "Service.h"

Service::Service()
{
    dispatcher_.registerHander(1, [this](const TcpConnectionPtr& conn, const json& js, Timestamp time)
    { this->ProcessingMessages(conn, js, time); });

}

