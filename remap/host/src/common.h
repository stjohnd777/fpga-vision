#pragma once

#define COLUMNS 2592
#define ROWS 1944
#define DEPTH 1
#define CHANNELS  1

#define QUEUE_DEPTH 10

#define SERVER_PORT  8080
#define WFOV 0

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



#include <string>

//std::string imgSGS_2592x1944x8x1Path = "data/sgs_2592x1944x8x1.png";
//std::string imgFLIR_2592x1944x16x1Path = "data/flir-2592x1944x16x1.png";


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

/**
 * This function
 *  - reads in mono8 image file
 *  - initializes the remap X and Y maps
 *  - performs cv::remap and saves img to file
 *  - Initializes the plRemap with our methodology
 *  - performs accelerated xf::remap ans saves imf to file
 *  - perfroms a diff on the two remaps
 */
void RunSimpleRemapMono8FileTest(std::string imagePath  ) ;
void RunSimpleRemapMono16FileTest(std::string imagePath  ) ;

/**
 * This function
 *  - reads in mono8 image file
 *  - initializes the remap X and Y maps
 *  - performs cv::remap N times with timming
 *  - Initializes with RunSimpleRemapMono8FileXilinxTest method
 *  - performs accelerated xf::remap N times with timing
 *  - perfroms a diff on the two remaps
 */
int RunSimpleRemapMono8FileXilinxTest(std::string imagePath, int N) ;
