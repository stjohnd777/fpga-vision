//
// Created by e438262 on 5/3/2023.
//

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

#include <opencv2/core.hpp>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "common.h"
using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

struct configs {
	PixelFormatEnums pfmt;
	unsigned int numberBufferers;
	float exposure;
	bool isLogging;
	int timeOutms;
};

template<typename T>
struct Response {
	bool isSuccess = false;
	T payload;
	int errorCode = -1;
	string errorMessage;
	std::time_t t;

};

typedef Response<ImagePtr> GetImageResponse;

class LoggingEventHandlerImpl: public LoggingEventHandler {
	void OnLogEvent(LoggingEventDataPtr loggingEventDataPtr);

	int m_LoggingLevel = 300;
};

GetImageResponse* SteroPair(int indexLeft, int indexRight);

class BlkFlyCameraManager {
public:
	static cv::Mat ConvertToCVMat(ImagePtr pImage);
	static void Init(PixelFormatEnums f, unsigned int numBuffers);
	static GetImageResponse GetImageFromCamera(int index, float exposure = 0);
	static unsigned int GetCameraCount();
	static bool IsInValidState(int index);
	static string GetDeviceInfo(int index = 0);
	static double GetExposure(int index = 0);
	static void SetExposure(int index, float exposure);
	static void SetAutoExposure(int index = 0);
	static void SetBufferCount(int index, unsigned int numBuffers);
	static unsigned int GetBufferCount(int index);
	static void Clear();
	static const int LOGGING_LEVEL;
	static bool LOGGING;
private:
	static SystemPtr s_SystemPtr;
	static CameraList s_CameraList;
	static LoggingEventHandlerImpl s_LoggingEventHandler;
};

unsigned int GetGigECameraCount();

cv::Mat ConvertToCVMat(ImagePtr pImage);

