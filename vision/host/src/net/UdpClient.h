#pragma once

#include "net.h"

template<class Req, class Res>
Res* RequestReply(string host, unsigned short port, Req req) {
    UdpUtils udpClient;
    std::tuple<size_t, std::shared_ptr<uint8_t[]>> tup = udpClient.RequestReply(host,port, Serialize(req),sizeof(Req));
    uint8_t* data = get<1>(tup).get();
    Res * res = DeSerialize<Res>(data);
    return res;
}

template<class Req, class Res>
class UdpClient {
public:
    UdpClient(std::string host, unsigned short port) : m_host(host), m_port(port) {
    }

    Res* RequestReply(Req req) {
        UdpUtils udpClient;
        std::tuple<size_t, std::shared_ptr<uint8_t[]>> tup = udpClient.RequestReply(m_host,m_port, Serialize(req),sizeof(Req));
        uint8_t* data = get<1>(tup).get();
        Res * res = DeSerialize<Res>(data);
        return res;
    }

protected:
    std::string m_host;
    unsigned short m_port;
};
 
