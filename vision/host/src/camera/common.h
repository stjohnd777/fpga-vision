
#pragma once

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
#include <boost/circular_buffer.hpp>
#include <opencv2/videoio.hpp>


#include "Camera.h"
#include "ImgProcDelegate.h"


using namespace std;
using namespace cv;


void drawLine( Mat img, Point start, Point end , int thickness = 2,  int lineType = LINE_8);

std::string psuedoRandomString(std::string::size_type length);

std::string showit(std::shared_ptr<cv::Mat> spMa) ;

extern "C" void displayCamera(int cameraID, bool wait = true) ;

extern "C" void displayImage(std::string path) ;
