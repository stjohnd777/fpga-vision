#pragma once
#include <opencv2/core/core.hpp>

void plThreshold(cv::Mat& in, cv::Mat& out, unsigned char thresh = 128, unsigned char maxval = 255) ;
