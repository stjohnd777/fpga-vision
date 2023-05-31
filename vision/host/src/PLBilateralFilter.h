#include "config/config.h"

#include <opencv2/core/core.hpp>


cv::Mat plBilateralFilter(
		cv::Mat& in,
		float sigma_color,
	    float sigma_space);

