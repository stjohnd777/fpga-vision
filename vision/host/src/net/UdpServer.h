#pragma once

#include "net.h"

template<class Request, class Response>
class UdpServer {
public:
    UdpServer(std::string host, unsigned short port) : m_host(host), m_port(port) {}

    void ReceiveReply(std::function<Response *(Request *)> handler) {

        using namespace boost::asio;
        using namespace boost::asio::ip;

        udp::socket socket(m_ios, boost::asio::ip::udp::endpoint( udp::v4(), m_port));

        uint8_t recv_buf[sizeof(Response)];
        udp::endpoint remote_endpoint;
        std::size_t bytes_received = socket.receive_from(buffer(recv_buf), remote_endpoint);

        uint8_t* p = new uint8_t[bytes_received];
        memcpy(p, recv_buf, bytes_received);
        Request* req = DeSerialize<Request>(p);

        Response* res = handler(req);
        uint8_t* resBytes = Serialize<Response>(*res);

        auto size_bytes_response = sizeof(Response);

        boost::system::error_code ignored_error;
        socket.send_to(buffer(resBytes,size_bytes_response), remote_endpoint, 0, ignored_error);
    }

    thread* Start(std::function<Response *(Request *)> handler) {
        return new thread([&](){
            m_IsRunning = true;
            while (m_IsRunning) {
                ReceiveReply(handler);
            }
        });
    }

    void Stop() {
        m_IsRunning = false;
    }

protected:
    boost::asio::io_service m_ios;
    bool m_IsRunning = false;
    std::string m_host;
    unsigned short m_port;
};
 
 
