#include "ImageServer.h"

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

#include <mutex>
#include <mutex>
std::mutex g_mutex;
boost::circular_buffer<Frame> circularBufferCamera0(10);
boost::circular_buffer<Frame> circularBufferCamera1(10);
boost::circular_buffer<Frame> circularBufferCamera2(10);
boost::circular_buffer<Frame> circularBufferCamera3(10);
boost::circular_buffer<Frame> circularBufferCamera4(10);

boost::circular_buffer<Frame> circularBufferCamera5(10);
boost::circular_buffer<Frame> circularBufferCamera6(10);
boost::circular_buffer<Frame> circularBufferCamera7(10);
boost::circular_buffer<Frame> circularBufferCamera8(10);

boost::circular_buffer<Frame> circularBufferCamera9(10);
boost::circular_buffer<Frame> circularBufferCameraSGS(10);

//uchar* GetImage(string imageSeed) {
//	Mat in = imread(imageSeed, IMREAD_GRAYSCALE);
//	uchar * img = new uchar[ROWS * COLUMNS * DEPTH];
//	memcpy(img, in.data, ROWS * COLUMNS * DEPTH);
//	return img;
//}


void QueueSGSFrame(Frame f) {
	circularBufferCameraSGS.push_back(f);
}

void QueueCameraFrame(Frame f) {

	std::lock_guard < std::mutex > guard(g_mutex);
	circularBufferCamera0.push_back(f);

	switch (f.cameraId) {
	case 1:
		circularBufferCamera1.push_back(f);
		break;
	case 2:
		circularBufferCamera2.push_back(f);
		break;
	case 3:
		circularBufferCamera3.push_back(f);
		break;
	case 4:
		circularBufferCamera4.push_back(f);
		break;
	case 5:
		circularBufferCamera5.push_back(f);
		break;
	default:
		// TODO: some error handling
		break;
	}
}



Frame LookUpCameraFrame(int cameraId, double time, int qNumber) {
	std::lock_guard < std::mutex > guard(g_mutex);
	Frame aFrame;
	switch (qNumber) {
	case 0:
		aFrame = circularBufferCamera0[0];
		break;
	case 1:
		aFrame = circularBufferCamera1[0];
		break;
	case 2:
		aFrame = circularBufferCamera2[0];
		break;
	case 3:
		aFrame = circularBufferCamera3[0];
		break;
	case 4:
		aFrame = circularBufferCamera4[0];
		break;
	default:
		// TODO: some error handling
		break;
	}
	return aFrame;
}

Frame LookUpSGSFrame( int qNumber) {
	std::lock_guard < std::mutex > guard(g_mutex);
	Frame aFrame  = circularBufferCameraSGS[qNumber];
	return aFrame;
}

bool isRunning = true;

thread* StartImageServer(unsigned short port) {

	thread* imageServer =
			new thread(
					[&]() {
						auto srv = new SyncTcpServer(  port);
						int service_count = 0;
						std::mutex l_mutex;

						while (isRunning) {

							srv->StartListeningAndHandleOneRequest( [&](int cameraId, double time) {
										Frame aFrame = LookUpCameraFrame(cameraId, time);
										std::lock_guard<std::mutex> guard(l_mutex);
										auto len = ROWS * COLUMNS * DEPTH;
										char* bytesToTransfer = new char[len];
										memset(bytesToTransfer,0,len);
										memcpy(bytesToTransfer, aFrame.img, ROWS * COLUMNS * DEPTH);
										auto tup = make_tuple(len, bytesToTransfer);
										return tup;
									});
							service_count++;
							cout << "server handled request " << service_count << endl;
						}
					});

	return imageServer;
}



