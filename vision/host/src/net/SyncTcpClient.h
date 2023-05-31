#pragma once

#include <boost/asio.hpp>

#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <functional>
#include <vector>

#include "../config/config.h"


using namespace boost;
using namespace std;

class SyncTcpClient {
public:
    SyncTcpClient(
            const std::string &host,
            unsigned short port)
            : m_ep(asio::ip::address::from_string(host), port),
              m_sock(m_ios) {
        m_sock.open(m_ep.protocol());
        m_sock.connect(m_ep);
    }

    std::tuple<size_t, const char *> request_reply(double time, uint16_t camera_id) {
        stringstream ss;
        ss << camera_id << "|" << time << endl;
        sendRequest(ss.str());
        return receiveResponse();
    };

    virtual ~SyncTcpClient() {
        close();
    }

private:

    virtual void sendRequest(const std::string &request) {
        asio::write(m_sock, asio::buffer(request));
    }

    std::tuple<size_t, const char *> receiveResponse() {
        boost::system::error_code error;
        boost::asio::streambuf receive_buffer;

        char *dst = new char[ROWS * COLUMNS * DEPTH];
        memset(dst, 0, ROWS * COLUMNS * DEPTH );
        size_t total = 0;
        while (true) {
            char temp_buffer[ROWS * COLUMNS * DEPTH];
            boost::system::error_code error;
            size_t len = m_sock.read_some(boost::asio::buffer(temp_buffer), error);

            memcpy(dst + total, temp_buffer, len);
            total += len;

            if (error == boost::asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                return make_tuple(0, nullptr);
            // throw boost::system::system_error(error);
        }
        return make_tuple(total, dst);
    }

    void close() {
        m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        m_sock.close();
    }

private:
    asio::io_service m_ios;
    asio::ip::tcp::endpoint m_ep;
    asio::ip::tcp::socket m_sock;
};
