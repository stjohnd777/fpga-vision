/*
 * MyImageServer.cpp
 *
 *  Created on: Apr 27, 2023
 *      Author: overman
 */

#include "MyImageServer.h"

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <vector>


#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>
#include <opencv2/core/core.hpp>
#include "common.h"

using namespace cv;
using namespace std;
using boost::asio::ip::tcp;


std::vector<std::string> mysplit(const std::string &str,const std::string &delim) {
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}






//////////////////////////////////////

MyImageServer::MyImageServer(int port, int cameraId) : port(port), cameraId(cameraId){
	buffer.set_capacity(QUEUE_DEPTH);
}


void MyImageServer::push_back(Frame f) {
	std::lock_guard < std::mutex > guard(g_mutex);
	buffer.push_back(f);
}
void MyImageServer::push_back(cv::Mat m) {
	std::lock_guard < std::mutex > guard(g_mutex);

	buffer.push_back(Frame(cameraId,m));
}

Frame MyImageServer::lookupByTime( double time ) {
	// TODO ( gps time from double to time_point then find duration )
	std::lock_guard < std::mutex > guard(g_mutex);
	Frame aFrame = buffer[0];
	return aFrame;
}



Frame MyImageServer::lookupByDepth( uint8_t depth ) {
	assert( depth <= QUEUE_DEPTH );
	std::lock_guard < std::mutex > guard(g_mutex);
	Frame aFrame = buffer[depth];
	return aFrame;
}



MyImageServer::~MyImageServer() {
	isRunning = false;
}


std::thread*  MyImageServer::Start() {

	isRunning = true;

	thread *ptrThread = new thread( [&,this](int port) {

		try {
			boost::asio::io_context io_context;
			tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
			while (true) {

				// Wait for a client to connect
				tcp::socket socket(io_context);
				acceptor.accept(socket);
				std::cout << "Client Connected" << std::endl;

				// GET REQUEST
				boost::asio::streambuf request_buf;
				boost::asio::read_until(socket, request_buf, "\n");
				std::string request = boost::asio::buffer_cast<const char *>(request_buf.data());

				// Parse Request
				std::cout << "Client Sent Request " << request << std::endl;
				auto tokens = mysplit(request, "|");

				Frame aFrame= lookupByDepth(0);

				boost::asio::write(socket, boost::asio::buffer(aFrame.img, aFrame.len));

				cout << "***********************************************" << endl;
				cout << "Width:" << COLUMNS << "X" << "Height:" << ROWS << ":" << DEPTH << endl;
				std::cout << "Image Bytes:" << aFrame.len << " Sent To Client " << std::endl;
			}
		}
		catch (std::exception &e) {
			std::cerr << "Exception: " << e.what() << std::endl;
			return;
		}
	}
	,  port);

	return ptrThread;
}

std::thread*  MyImageServer::Start(std::function<Frame(std::string)> handler ) {

	isRunning = true;

	thread *ptrThread = new thread( [&,this](int port) {

		try {
			boost::asio::io_context io_context;
			tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
			while (true) {

				// Wait for a client to connect
				tcp::socket socket(io_context);
				acceptor.accept(socket);
				std::cout << "Client Connected" << std::endl;

				// GET REQUEST
				boost::asio::streambuf request_buf;
				boost::asio::read_until(socket, request_buf, "\n");
				std::string request = boost::asio::buffer_cast<const char *>(request_buf.data());


				Frame aFrame= handler(request);

				boost::asio::write(socket, boost::asio::buffer(aFrame.img, aFrame.len));

				std::cout << "Image Bytes:" << aFrame.len << " Sent To Client " << std::endl;
			}
		}
		catch (std::exception &e) {
			std::cerr << "Exception: " << e.what() << std::endl;
			return;
		}
	}
	,  port);

	return ptrThread;
}

