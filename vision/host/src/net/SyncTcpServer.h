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
#include <tuple>

using namespace boost;
using namespace std;

struct camera_trigger {
	double time { 0 };
	uint16_t camera_id { 0 };
};

typedef std::function<std::tuple<size_t, char *>(int cameraId, double timestamp)> TCP_HANDLER;

class SyncTcpServer {

private:

	camera_trigger ParseRequest(string line) {
		camera_trigger ret;

		vector<string> tokens;
		string delimiter = "|";
		size_t pos = 0;
		while ((pos = line.find(delimiter)) != std::string::npos) {
			string token = line.substr(0, pos);
			tokens.push_back(token);
			line.erase(0, pos + delimiter.length());
		}
		tokens.push_back(line);
		ret.camera_id = stoi(tokens[0]);
		ret.time = stod(tokens[1]);

		return ret;
	}
public:


	SyncTcpServer(unsigned short port) :
	  m_port(port)
	, m_stop(false)
	{
		m_Acceptor = new asio::ip::tcp::acceptor(m_ios,asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port));

	}

	char *StartListeningAndHandleOneRequest( std::function<std::tuple<size_t, char *>(int cameraId, double timestamp)> handler) {

		cout << "server create socket" << endl;
		asio::ip::tcp::socket sock(m_ios);
		cout << "server accept on socket ... waiting/blocking" << endl;
		m_Acceptor->accept(sock);

		cout << "server accepted reading request" << endl;
		asio::streambuf request;
		asio::read_until(sock, request, '\n');
		std::istream is(&request);
		std::string line;
		std::getline(is, line);
		cout << "server read request " << line << endl;

		auto req = ParseRequest(line);

		size_t len = 0;
		char *image = nullptr;
		std::tuple<size_t, char *> tup;
		try {
			tup = handler(req.camera_id, req.time);
			len = get < 0 > (tup);
			image = get < 1 > (tup);

			cout << "server write to client " << len << " bytes" << endl;
			asio::write(sock, asio::buffer(image, len));
		} catch (std::exception &e) {
			cout << "Error in Handler " << e.what() << endl;
		}
		return image;
	}

private:
	asio::io_service m_ios;
	asio::ip::tcp::acceptor* m_Acceptor;
	std::atomic<bool> m_stop;
	unsigned short m_port;
};

