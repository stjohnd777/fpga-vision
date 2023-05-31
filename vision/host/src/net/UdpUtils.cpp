#include "UdpUtils.h"

#include <algorithm>
#include <boost/asio.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

#define  MAX_DATAGRAM  10000

boost::asio::ip::udp::endpoint GetUdpEndpoint(std::string host, unsigned short port) {
    boost::system::error_code ec;
    // checks whether argument contains a valid IPv4 or IPv6 address then
    // instantiates a corresponding v4 || v6
    auto ip_address = boost::asio::ip::address::from_string(host, ec);
    if (ec.value() != 0) {
        std::cout << ec.value() << std::endl;
        throw ec.value();
    }
    auto ep = boost::asio::ip::udp::endpoint(ip_address, port);
    return ep;
}

UdpUtils::UdpUtils() :  m_udp_socket(m_ios) {
    m_udp_socket.open(asio::ip::udp::v4());
}

void UdpUtils::Send(std::string host, unsigned short port, const uint8_t *pdata, size_t len) {
    auto udp_ep = GetUdpEndpoint(host, port);

    char data[MAX_DATAGRAM];
    memcpy(data, pdata, len);
    auto bufferedData = asio::buffer(data, len);
    m_udp_socket.send_to(bufferedData, udp_ep);
}

std::tuple<size_t, std::shared_ptr<uint8_t[]>>
UdpUtils::ServerReceiveNoReply(std::string host, unsigned short port) {

    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));

    uint8_t recv_buf[MAX_DATAGRAM] = {0};

    boost::asio::ip::udp::endpoint remote_endpoint;
    std::size_t bytes_received = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

    std::shared_ptr<uint8_t[]> sp(new uint8_t[bytes_received]);
    memcpy(sp.get(), recv_buf, bytes_received);

    return std::make_tuple(bytes_received, sp);
}

std::tuple<size_t, std::shared_ptr<::uint8_t[]>>
UdpUtils::RequestReply(std::string host, unsigned short port, const uint8_t *pdata, size_t len) {
    Send(host, port, pdata, len);
    uint8_t recv_buf[MAX_DATAGRAM] = {0};

    boost::asio::ip::udp::endpoint sender_endpoint;
    size_t bytes_received = m_udp_socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

    std::shared_ptr<uint8_t[]> sp(new uint8_t[bytes_received]);
    memcpy(sp.get(), recv_buf, bytes_received);

    return std::make_tuple(bytes_received, sp);
}

void UdpUtils::ReceiveReply(std::string host, unsigned short port, REQ_REPLY_HANDLER f) {
    using namespace boost::asio;
    using namespace boost::asio::ip;

    udp::socket socket(m_ios, boost::asio::ip::udp::endpoint(udp::v4(), port));

    uint8_t recv_buf[MAX_DATAGRAM];
    udp::endpoint remote_endpoint;

    std::size_t bytes_received = socket.receive_from(buffer(recv_buf), remote_endpoint);
    std::shared_ptr<uint8_t[]> sp(new uint8_t[bytes_received]);

    memcpy(sp.get(), recv_buf, bytes_received);

    auto req = std::make_tuple(bytes_received, sp);

    auto res = f(req);



    auto size_bytes_response = std::get<0>(res);
    auto spBytes = std::get<1>(res);
    uint8_t *pBytes = spBytes.get();
    uint8_t send_buf[size_bytes_response];
    memcpy(send_buf, pBytes, size_bytes_response);

    boost::system::error_code ignored_error;
    socket.send_to(buffer(send_buf, size_bytes_response), remote_endpoint, 0, ignored_error);
}


UdpUtils::~UdpUtils() {
    try {
        m_udp_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        m_udp_socket.close();
    }catch(...){

    }
}




