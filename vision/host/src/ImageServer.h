#pragma once

#include <thread>
#include <string>
#include <stdlib.h>
#include <chrono>

#include "config/config.h"
#include "camera/Camera.h"
#include "net/net.h"
#include "PLThreshold.h"


#include <boost/circular_buffer.hpp>
#include <opencv2/core/core.hpp>
#include "PLMyThresholding.h"

using namespace cv;

struct Frame {
	Frame() : seq(0), gpsTime(0){
		memset(img, 0, ROWS * COLUMNS * DEPTH);
		len =  ROWS * COLUMNS * DEPTH;
	}
	Frame(uint32_t cameraId,Mat in) :cameraId(cameraId) {
		memcpy(img, in.data, ROWS * COLUMNS * DEPTH);
		len =  ROWS * COLUMNS * DEPTH;
	}
	uint32_t seq;
	double gpsTime;
	uint8_t cameraId;
	char img[ROWS * COLUMNS * DEPTH];
	size_t len;
};



uchar* GetImage(string imageSeed = "data/ori.jpg") ;

void QueueSGSFrame(Frame f);

void QueueCameraFrame(Frame f);

Frame LookUpSGSFrame( int qNumber) ;

Frame LookUpCameraFrame(int cameraId, double time,int qNumber =0);

std::thread* StartImageServer( unsigned short port) ;

void StartCamera(int cameraId) ;


