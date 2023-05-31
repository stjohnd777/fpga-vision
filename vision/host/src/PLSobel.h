#pragma once
#include <opencv2/core/core.hpp>


void cvSobel(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width );
void plSobel(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width );
