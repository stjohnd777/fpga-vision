#pragma once
#include <opencv2/core/core.hpp>

#include "config.h"


void cvSobel(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width );

void plSobel8(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width );

void plSobel16(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width );
