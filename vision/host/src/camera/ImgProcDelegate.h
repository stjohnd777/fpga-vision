#pragma once

#include "common.h"

class ImgProcDelegate {

public:

	ImgProcDelegate();

	static cv::Mat IDENTITY(cv::Mat m);

	static cv::Mat cvToGrayScaleHandler(cv::Mat m) ;

	static cv::Mat cvDiff(cv::Mat m1,cv::Mat m2);

	static cv::Mat cvBlurHandler(cv::Mat m) ;

	static cv::Mat cvBlur(cv::Mat m,int kernel_size, double sigmaX, double sigmaY) ;

	static cv::Mat cvDilateHandler(cv::Mat m);

	static cv::Mat cvDilate(cv::Mat m,double thresh, double maxval, int type, cv::Point anchor,int iterations) ;

	static void cvFast(cv::Mat m,std::vector<cv::KeyPoint> &keypoints);

	virtual ~ImgProcDelegate();
};



boost::circular_buffer< cv::Mat >  tracker(boost::circular_buffer<  cv::Mat > buffer);

cv::Mat tracker(cv::Mat frame0, cv::Mat frame1) ;
