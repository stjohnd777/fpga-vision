#pragma once

#include "net.h"
#include <algorithm>
#include <thread>

using namespace std;

template<class Req>
class UdpConsumer {
public:
    UdpConsumer(std::string host, unsigned short port) : m_host(host), m_port(port) {}

    Req * ReceiveOne() {
        std::tuple<size_t,std::shared_ptr<uint8_t[]>> tup = m_UdpUtils.ServerReceiveNoReply(m_host, m_port);
        uint8_t* data = get<1>(tup).get();
        Req *req = DeSerialize<Req>(data);
        return req;
    }

    thread* Listen(std::function<void(Req *)> handler) {
        m_IsRunning = true;
        return new thread([&] (){
            while (m_IsRunning) {
                Req* req = ReceiveOne();
                handler(req);
            }
        });
    }

    void Stop() {
        m_IsRunning = false;
    }

private:
    std::string m_host;
    unsigned short m_port;
    bool m_IsRunning = false;
    UdpUtils m_UdpUtils;
};
 
