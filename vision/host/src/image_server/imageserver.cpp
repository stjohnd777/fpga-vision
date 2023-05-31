/*
 * imageserver.cpp
 *
 *  Created on: Apr 10, 2023
 *      Author: overman
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>


#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <boost/asio.hpp>


using namespace std;
using namespace cv;
using boost::asio::ip::tcp;

#include "imageserver.h"

std::vector<std::string> split(const std::string &str, const std::string &delim) {
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


bool CheckShutdown(std::thread::id id) {
    // TODO: poll file system for stop file
    std::lock_guard<std::mutex> guard(g_mutexImageServers);
    return mapImageServers[id];
}


thread *startImageServer(int cameraId , int port ) {


    thread *ptrThread = new thread([&](int cameraId, int port) {
        try {

            mapImageServers[this_thread::get_id()] = true;
            bool isCamera = true;
            VideoCapture *pVideoCapture = new VideoCapture(cameraId);
            if (!pVideoCapture->isOpened()) {
                mapImageServers[this_thread::get_id()] = false;
            }
            if (!pVideoCapture->isOpened()) {
                delete pVideoCapture;
                mapImageServers[this_thread::get_id()] = false;
            }

            boost::asio::io_context io_context;
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

            while (!CheckShutdown(this_thread::get_id())) {

                // Wait for a client to connect
                tcp::socket socket(io_context);
                acceptor.accept(socket);

                std::cout << "Client Connected" << std::endl;

                // GET REQUEST
                boost::asio::streambuf request_buf;
                boost::asio::read_until(socket, request_buf, "\n");
                std::string request = boost::asio::buffer_cast<const char *>(request_buf.data());
                auto tokens = split(request, "|");

                // GET CAMERA FRAME ... Likely Needs Mutex on Get Camera Frame
                cv::Mat aCameraFrame;
                *(pVideoCapture) >> aCameraFrame;

                // COLOR CONVERT (TEMP ... UNTIL MONO)
                cvtColor(aCameraFrame, aCameraFrame, COLOR_BGR2GRAY);

                size_t len = aCameraFrame.rows * aCameraFrame.cols * aCameraFrame.elemSize();

                boost::asio::write(socket, boost::asio::buffer(aCameraFrame.data, len));

                std::cout << "Image Bytes:" << len << " Sent To Client " << std::endl;
            }
        }
        catch (std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            mapImageServers[this_thread::get_id()] = false;

        }
    }, cameraId, port);

    return ptrThread;
}

