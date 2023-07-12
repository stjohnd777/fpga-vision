#include "common.h"
#include "myutils.h"
#include "PLRemap.h"



using namespace std;
using namespace cv;

PixelFormatEnums Configs::pixelFmt = PixelFormatEnums::PixelFormat_Mono16;
unsigned int Configs::numBuffers = 2;
bool Configs::isWriteFile = true;
string Configs::imgfmt = ".pgm";
string Configs::imgDir = "/data/nodejs/app/public";
bool Configs::isImageRaw = true;
bool Configs::isRemap = false;
bool Configs::isCameraGaurd = false;

/*
 * Using different cameras in default mode
 */
cv::Mat cameraGuard(cv::Mat in) {
	if (in.depth() != 0) {
		cvtColor(in, in, COLOR_RGB2GRAY);
		cout << "Camera Guard: Color convert to grey" << endl;
	}
	if (in.rows != ROWS || in.cols != COLUMNS) {
		cv::resize(in, in, Size(COLUMNS, ROWS), cv::INTER_LINEAR);
		cout << "Camera Guard: resize(" << ROWS << "," << COLUMNS << ")"
				<< endl;
	}
	return in;
}

// TODO: sleep time in properties file
void mysleep(unsigned int ms) {
	this_thread::sleep_for(chrono::milliseconds(ms));
}



#include <thread>
#include <string>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <sstream>
#include <tuple>
#include <time.h>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <boost/circular_buffer.hpp>

#include <CL/cl2.hpp>
#include "BlkFlyCameraManager.h"
#include "PLRemap.h"


#include "common.h"
#include "MyImageServer.h"
#include "OpenCvCamera.h"

using namespace std;
using namespace cv;

int numberFrames = 10;
bool isCameraGaurd = false;
bool isWriteFile = true;
bool isImageRaw = true;
bool isRemap = false;
int usbCameraId = 0;
std::string imgSGS_2592x1944x8x1Path = "data/sgs_2592x1944x8x1.png";
std::string imgFLIR_2592x1944x16x1Path = "data/flir-2592x1944x16x1.png";

