
#include "ImgProcDelegate.h"
#include "common.h"

#include <opencv2/features2d.hpp>

ImgProcDelegate::ImgProcDelegate() {
	// TODO Auto-generated constructor stub

}


cv::Mat ImgProcDelegate::IDENTITY(cv::Mat m) {
	if ( DEBUG) {
		static std::string namedWindow = "IDENTITY";
        DisplayImageInWindow(namedWindow, m);
	}
	return m;
}


cv::Mat ImgProcDelegate::cvToGrayScaleHandler(cv::Mat m) {
	cv::Mat grey ;
	cvtColor(m.clone(), grey, cv::COLOR_BGR2GRAY);
	if ( DEBUG) {
		static std::string namedWindow = "cvtColor(COLOR_BGR2GRAY)";
        DisplayImageInWindow(namedWindow, grey);
	}
	return grey;
}


cv::Mat ImgProcDelegate::cvDiff(cv::Mat m0, cv::Mat m1) {
	cv::Mat diff ;
	cv::absdiff(m1, m0, diff);
	if ( DEBUG) {
		static std::string namedWindow = "absdiff";
        DisplayImageInWindow(namedWindow, diff);
	}
	return diff;
}


cv::Mat ImgProcDelegate::cvBlurHandler(cv::Mat m) {
	return cvBlur(m, 3, .5, .5);
}


cv::Mat ImgProcDelegate::cvBlur(cv::Mat m, int kernel_size, double sigmaX, double sigmaY) {

	int borderType = cv::BORDER_DEFAULT;
	cv::Size kernel(kernel_size, kernel_size);
	cv::Mat blur ;
	cv::GaussianBlur(m.clone(), blur, kernel, sigmaX, sigmaY, borderType);

	if ( DEBUG) {
		static std::string namedWindow = "GaussianBlur(" + to_string(kernel_size) + "," + to_string(sigmaX ) + "," + to_string(sigmaY) + ")";
        DisplayImageInWindow(namedWindow, blur);
	}
	return blur ;
}


cv::Mat ImgProcDelegate::cvDilateHandler(cv::Mat m) {
	return cvDilate(m, 20, 255, cv::THRESH_BINARY, cv::Point(-1, -1), 1);
}

cv::Mat ImgProcDelegate::cvDilate(cv::Mat m, double thresh, double maxval, int type, cv::Point anchor, int iterations) {
	cv::Mat kernel;
	cv::Mat spDilate ;
	const cv::Scalar &borderValue = cv::morphologyDefaultBorderValue();
	cv::dilate(m.clone(), spDilate, kernel, anchor, iterations, cv::BORDER_CONSTANT, borderValue);

	if ( DEBUG) {
		static std::string namedWindow = "dilate(" + to_string(thresh) + "," + to_string(maxval ) + "," + to_string(type) + "," +to_string(iterations) +")";
        DisplayImageInWindow(namedWindow, spDilate);
	}
	return spDilate;
}

void ImgProcDelegate::cvFast(cv::Mat m, std::vector<cv::KeyPoint> &keypoints){
     // TODO
	int threshold = 20;
	cv::FAST(m, keypoints, threshold);
}


ImgProcDelegate::~ImgProcDelegate() {
	// TODO Auto-generated destructor stub
}





// diff
// grey
// Dilate
// find findContours
// draw in mat

cv::Mat tracker( cv::Mat frame0, cv::Mat frame1) {

    auto grey0 = ImgProcDelegate::cvToGrayScaleHandler(frame0);
    auto grey1 = ImgProcDelegate::cvToGrayScaleHandler(frame1);

    auto blur0 = ImgProcDelegate::cvBlurHandler(grey0) ;
    auto blur1 = ImgProcDelegate::cvBlurHandler(grey1) ;

    auto diff01 = ImgProcDelegate::cvDiff(grey0,grey1);
    auto diff10 = ImgProcDelegate::cvDiff(grey1,grey0);

    auto diff = ImgProcDelegate::cvDiff(diff01,diff10);

    auto blur = ImgProcDelegate::cvBlurHandler(diff) ;
    //auto dilate = ImgProcDelegate::cvDilateHandler(blur);

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(blur, contours, hierarchy,cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    // draw them
    auto green = cv::Scalar(0, 255, 0);
    cv::InputArrayOfArrays loops = contours;
    cv::drawContours(frame0, loops, -1, green, 1);// , cv::LINE_8, cv::noArray(), cv::INTER_MAX, cv::Point());

    return frame0;
}

boost::circular_buffer< cv::Mat > tracker(boost::circular_buffer< cv::Mat > buffer) {

    auto frame0 = buffer[0];
    auto frame1 = buffer[1];
    auto frame3 = tracker(frame0,frame1);

    boost::circular_buffer< cv::Mat>  ret;
    ret.set_capacity(2);
    ret.push_back(frame3);
    return ret;
}
