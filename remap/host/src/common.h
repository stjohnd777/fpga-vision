#pragma once

#define COLUMNS 2592
#define ROWS 1944
#define DEPTH 2
#define CHANNELS  1

#define QUEUE_DEPTH 10

#define SERVER_PORT  8080

#define GETTERSETTER(varType, varName, funName)\
    protected: varType varName ;\
    public: virtual varType get##funName(void) const {\
        return varName;\
    }\
    public: virtual void set##funName(varType v){\
        varName =v;\
    }\

#define ATTRV(varTy1pe, varName, funName, v)\
    protected: varType varName = v ;\
    public: virtual varType get##funName(void) const {\
        return varName;\
    }\
    public: virtual void set##funName(varType var){\
        varName = var;\
    }\

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

struct Configs {
	static PixelFormatEnums pixelFmt;
	static unsigned int numBuffers;
	static bool isWriteFile;
	static string imgfmt ;
	static string imgDir ;
	static bool isImageRaw ;
	static bool isRemap ;
	static bool isCameraGaurd;
};


#include <string>


#include <opencv2/core.hpp>
/*
 * Using different cameras in default mode
 */
cv::Mat cameraGuard(cv::Mat in) ;

#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <thread>

#include "common.h"
#include "PLRemap.h"

using namespace std;
using namespace cv;


// TODO: sleep time in properties file
void mysleep(unsigned int ms = 1000) ;

