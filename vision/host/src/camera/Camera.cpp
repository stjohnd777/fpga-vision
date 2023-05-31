
#include "Camera.h"


#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <exception>
#include <opencv2/opencv.hpp>
#include <boost/circular_buffer.hpp>


using namespace std;
using namespace cv;



void DisplayImageInWindow(std::string& namedWindow, cv::Mat  spMat){
    cv::imshow(namedWindow, spMat);
}


Camera::Camera(int cameraID, int bufferDepth) {
	this->isCapturing = true;
	this->cameraID = cameraID;
	this->buffer_depth = bufferDepth;

	circular_buffer.set_capacity(buffer_depth);

	pVideoCapture = new VideoCapture(cameraID);

	// Validate Camera Found
	if (!pVideoCapture->isOpened()) {
		cout << "A fatal error  pVideoCapture is not opened" << endl;
		throw new runtime_error("VideoCapture Failure");
	}
	// Validate Can Read VCamera
	cv::Mat imageFrame;
	*(this->pVideoCapture) >> imageFrame;
	if (imageFrame.empty()) {
		throw new runtime_error("GetFrame Failure");
	} else {
		cout << "GetFrame Success" << endl;
	}
}

Mat Camera::GetFrame() {
	cv::Mat frame ;
	*(this->pVideoCapture) >> frame;
	if (frame.empty()) {
		this->isCapturing = false;
		throw new runtime_error("Camera::GetFrame() is empty");
	}
	this->circular_buffer.push_back(frame);
	return frame;
}

void Camera::AddSequentialFrameProcessor(frame_handler f) {
	if (f == nullptr) {
		throw new runtime_error("Camera::Start(frame_handler f) has empty frame handler");
	}
	m_vFrameHandlers.push_back(f);
}

void Camera::AddSequentialBufferProcessor(buffer_handler f) {
	if (f == nullptr) {
		throw new runtime_error("Camera::Start(frame_handler f) has empty frame handler");
	}
	m_vBufferHandlers.push_back(f);
}

void Camera::Start() {

	try {
		while (this->isCapturing) {
			auto spImageFrame = this->GetFrame();

			for (auto f : this->m_vFrameHandlers) {
				spImageFrame = f(spImageFrame);
			}
			for (auto f : this->m_vBufferHandlers) {
				f(this->circular_buffer);
			}
		}
	} catch (...) {
		cout << "Graceful Catch is processing frames " << endl;
	}
	cout << "Camera Capture Thread Exiting " << endl;

}

void Camera::StartThread() {

	thread video_capture_thread([&]() {
		try {
			while (this->isCapturing) {
				auto spImageFrame = this->GetFrame();
				for (auto f : this->m_vFrameHandlers) {
					spImageFrame = f(spImageFrame);
				}

				for (auto f : this->m_vBufferHandlers) {
					f(this->circular_buffer);
				}
			}
		} catch (exception& e) {
			cout << "Graceful Catch is processing frames " << endl;
			cout << e.what() << endl;
		}
		cout << "Camera Capture Thread Exiting " << endl;
	});
	video_capture_thread.detach();
}


cv::Mat Camera::GetPreviousFrame(int depth ){

	if ( circular_buffer.size() > depth){
	  return circular_buffer[depth];
	}
	throw std::runtime_error("std::shared_ptr<cv::Mat> GetPreviousFrame invalid depth");
}

int Camera::getBufferedFrameDepth(){
    return circular_buffer.size();
}


Camera::~Camera() {
	this->isCapturing = false;
	pVideoCapture->release();
	delete pVideoCapture;
	destroyAllWindows();
}


