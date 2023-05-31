#pragma once

#include "net.h"

template<class Req>
void Send(string host, unsigned short port, Req &req) {
    UdpUtils udpClient;
    udpClient.Send(host, port, Serialize(req), sizeof(req));
}


