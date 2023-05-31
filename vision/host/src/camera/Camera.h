#pragma once

#include "common.h"
#include "ImgProcDelegate.h"

typedef std::function<cv::Mat(cv::Mat)> frame_handler;
typedef std::function<boost::circular_buffer< cv::Mat> (boost::circular_buffer<cv::Mat>)> buffer_handler;

void DisplayImageInWindow(std::string& namedWindow,  cv::Mat m);

class Camera {
	GETTERSETTER(int, cameraID,CameraID)
	GETTERSETTER(bool, isCapturing, IsCaputing)
	GETTERSETTER(cv::VideoCapture *, pVideoCapture, VideoCapture)
	GETTERSETTER(int, buffer_depth, BufferDepth)
public:
	static  Camera* GetCamera(int cameraID, int bufferDepth = 10) {
        Camera* camera = new Camera( cameraID ,  bufferDepth);
		return camera;
	}

	Camera(int cameraID , int bufferDepth);
	void AddSequentialFrameProcessor(frame_handler f);
	void AddSequentialBufferProcessor(buffer_handler f);

	cv::Mat GetFrame();

	void Start();
	void StartThread();

	cv::Mat GetPreviousFrame(int depth =0);

	int getBufferedFrameDepth();

	virtual ~Camera();

private:
	boost::circular_buffer< cv::Mat > circular_buffer;
	std::vector<frame_handler> m_vFrameHandlers;
	std::vector<buffer_handler> m_vBufferHandlers;
};



