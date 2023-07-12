#pragma once

#include "common.h"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

#include <boost/circular_buffer.hpp>

class OpenCvCamera {

public:
	OpenCvCamera(int cameraID, int bufferDepth = 10);
	virtual cv::Mat GetFrame();
	cv::Mat GetPreviousFrame(size_t depth = 0);
	int getBufferedFrameDepth();
	virtual ~OpenCvCamera();
private:
	boost::circular_buffer<cv::Mat> circular_buffer;
	int cameraID;
	bool isCapturing;
	cv::VideoCapture* pVideoCapture;
	int buffer_depth;

};

