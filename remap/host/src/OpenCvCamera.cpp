#include "OpenCvCamera.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <exception>
#include <opencv2/core.hpp>
#include <boost/circular_buffer.hpp>

#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
using namespace std;
using namespace cv;

OpenCvCamera::OpenCvCamera(int cameraID, int bufferDepth) {
	this->isCapturing = true;
	this->cameraID = cameraID;
	this->buffer_depth = bufferDepth;

	circular_buffer.set_capacity(buffer_depth);

	pVideoCapture = new VideoCapture(cameraID);

	// Validate OpenCvCamera Found
	if (!pVideoCapture->isOpened()) {
		cout << "A fatal error  pVideoCapture is not opened" << endl;
		throw new runtime_error("VideoCapture Failure");
	}
	// Validate Can Read OpenCvCamera
	cv::Mat imageFrame;
	*(this->pVideoCapture) >> imageFrame;
	if (imageFrame.empty()) {
		throw new runtime_error("GetFrame Failure");
	} else {
		cout << "GetFrame Success" << endl;
	}
}

Mat OpenCvCamera::GetFrame() {
	cv::Mat frame;
	*(this->pVideoCapture) >> frame;
	if (frame.empty()) {
		this->isCapturing = false;
		throw new runtime_error("OpenCvCamera::GetFrame() is empty");
	}
	this->circular_buffer.push_back(frame);
	return frame;
}

cv::Mat OpenCvCamera::GetPreviousFrame(int depth) {

	if (circular_buffer.size() > depth) {
		return circular_buffer[depth];
	}
	throw "bad depth";
}

int OpenCvCamera::getBufferedFrameDepth() {
	return circular_buffer.size();
}

OpenCvCamera::~OpenCvCamera() {
	this->isCapturing = false;
	pVideoCapture->release();
	delete pVideoCapture;
}

