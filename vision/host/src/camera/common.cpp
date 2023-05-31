
#include "common.h"

#define DEBUG 1

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

#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <algorithm>
#include <chrono>


#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <boost/circular_buffer.hpp>


#include "Camera.h"
#include "ImgProcDelegate.h"


using namespace std;
using namespace cv;


void drawLine( Mat img, Point start, Point end , int thickness ,  int lineType )
{
    line( img,
          start,
          end,
          Scalar( 0, 0, 0 ),
          thickness,
          lineType );
}

static const std::string dictonary ="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

std::string psuedoRandomString(std::string::size_type length) {
    auto dictonary_length = dictonary.length() - 1;
    std::string random_string;
    random_string.reserve(length);
    for (unsigned int i=0; i < length ; i++) {
        int aRandomInt = rand();
        unsigned int random_index = aRandomInt % dictonary_length;
        random_string += dictonary.at(random_index);
    }
    return random_string;
}

std::string showit(std::shared_ptr<cv::Mat> spMa) {
    auto name = psuedoRandomString(10);
    namedWindow(name, cv::WINDOW_AUTOSIZE);
    cv::imshow(name, *spMa);
    return name;
}

extern "C" void displayCamera(int cameraID, bool wait) {

    std::string window_name = "Mode 1 Camera:0 >";

    VideoCapture *pVideoCapture = new VideoCapture(0);

    if (!pVideoCapture->isOpened()) {
        throw new runtime_error("VideoCapture Failure");
    }

    namedWindow(window_name, WINDOW_AUTOSIZE);

    if (!pVideoCapture->isOpened()) {
        delete pVideoCapture;
        throw new runtime_error("VideoCapture Failure");
    }

    cv::Mat imageFrame;

    while (1) {
        *(pVideoCapture) >> imageFrame;
        imshow(window_name, imageFrame);
        if (imageFrame.empty()) {
            break;
        }
        if (waitKey(10) == 27) {
            break;
        }
    }
    pVideoCapture->release();
    destroyAllWindows();
}

extern "C" void displayImage(std::string path) {

    Mat image;
    image = imread(path, 1);

    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", image);

    waitKey(0);
}
