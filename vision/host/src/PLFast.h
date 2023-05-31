#pragma once
#include <opencv2/core/core.hpp>

cv::Mat cvFast(cv::Mat& in_gray, float threshold, int rows, int cols);
cv::Mat plFast(cv::Mat& in, unsigned char threshold, int rows, int cols) ;

//std::vector<cv::Point> extract(cv::Mat out_hls) ;
