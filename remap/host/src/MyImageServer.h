/*
 * MyImageServer.h
 *
 *  Created on: Apr 27, 2023
 *      Author: overman
 */

#pragma once

#include "common.h"

#include <string>
#include <mutex>
#include <thread>
#include <functional>
#include <chrono>

#include <opencv2/core/core.hpp>

#include <boost/circular_buffer.hpp>

struct Frame {

	Frame() :cameraId(0) {
		memset(img, 0, ROWS * COLUMNS * DEPTH);
		len =  ROWS * COLUMNS * DEPTH;
		gpsTime = std::chrono::system_clock::now();

	}
	Frame(uint8_t cameraId,cv::Mat in) :cameraId(cameraId) {
		memcpy(img, in.data, ROWS * COLUMNS * DEPTH);
		len =  ROWS * COLUMNS * DEPTH;
		gpsTime = std::chrono::system_clock::now();
	}

	// TODO C+++ 20 gps_clock
	std::chrono::time_point<std::chrono::system_clock> gpsTime;

	uint8_t cameraId;

	size_t len;
	char img[ROWS * COLUMNS * DEPTH];
};



class MyImageServer {
public:

	MyImageServer( int port, int cameraId );

	Frame lookupByDepth( uint8_t depth );
	Frame lookupByTime( double time );


    void  push_back (Frame f);
    void  push_back (cv::Mat m);

	std::thread * Start();

	std::thread * Start(std::function<Frame(std::string)> handler);

	virtual ~MyImageServer();

private:

	int port;
	int cameraId;

private:

	boost::circular_buffer<Frame> buffer   ;
	std::mutex g_mutex;
	bool isRunning = false;
};

