#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <memory>
#include <tuple>
using namespace std;


#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
using namespace cv;


#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1

double getTimestamp();

#include <CL/cl2.hpp>

cl::Device GetDevice(const char* match ="Xilinx") ;
cl::Context GetContext(cl::Device& device);
size_t ReadFile(const string fileName, char *buf) ;
cl::Kernel GetKernel(cl::Device& device, cl::Context& context, const string& xclbinFilename, const string& kernelName) ;
std::tuple<cl::Device,cl::Context,cl::CommandQueue,cl::Kernel> init(std::string kernelName, std::string xclbinName);
