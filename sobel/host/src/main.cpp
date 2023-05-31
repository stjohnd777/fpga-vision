#include <boost/asio.hpp>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

#include "PLSobel.h"
#include "config.h"

using namespace std;
using namespace cv;




int main(int arc, char** argv) {

	cout << "Enter main() " << endl;


	string pathtoimg8 =   "data/sgs.2592x1944x8.png";
	auto in8 = imread(pathtoimg8, 0);
	if (!in8.data) {
		cout << "Failed to load the image ... !!!\n " << pathtoimg8 << endl;
		return -1;
	}

	cout << "---------------------------" << endl;

	Mat _grad_y8(ROWS, COLS, in8.depth());
	Mat _grad_x8(ROWS, COLS, in8.depth());
	cvSobel(in8, _grad_x8, _grad_y8, ROWS, COLS);
	imwrite("cv_grad_x8.jpg", _grad_x8);
	imwrite("cv_grad_y8.jpg", _grad_y8);


	Mat grad_y8(ROWS, COLS, in8.depth());
	Mat grad_x8(ROWS, COLS, in8.depth());
	plSobel8(in8, grad_x8, grad_y8, ROWS, COLS);
	imwrite("grad_x8.jpg", grad_x8);
	imwrite("grad_y8.jpg", grad_y8);
	cout << "plSobel8() done" << endl;
	cout << "---------------------------" << endl;



	string pathtoimg16 =   "data/flir.2592x1944.16.png";
	auto in16 = imread(pathtoimg16, 0);
	if (!in8.data) {
		cout << "Failed to load the image ... !!!\n " << pathtoimg16 << endl;
		return -1;
	}

	cout << "---------------------------" << endl;

	Mat _grad_y16(ROWS, COLS, in16.depth());
	Mat _grad_x16(ROWS, COLS, in16.depth());
	cvSobel(in16, _grad_x16, _grad_y16, ROWS, COLS);
	imwrite("cv_grad_x16.jpg", _grad_x16);
	imwrite("cv_grad_y16.jpg", _grad_y16);

	Mat grad_y16(ROWS, COLS, in8.depth());
	Mat grad_x16(ROWS, COLS, in8.depth());
	plSobel8(in16, grad_x16, grad_y16, ROWS, COLS);
	imwrite("grad_x16.jpg", grad_x16);
	imwrite("grad_y16.jpg", grad_y16);
	cout << "plSobel16() done" << endl;
	cout << "---------------------------" << endl;


//	try {
//
//		FLIRCameraController::BUFFER_COUNT = 10;
//		FLIRCameraController::defaultPixelFormat = PixelFormatEnums::PixelFormat_Mono8;
//		FLIRCameraController::LOGGING = false;
//
//		for (auto i = 0; i < 10; i++) {
//
//			GetImageResponse res = FLIRCameraController::GetInstance()->GetImage(7000);
//
//			if (res.isSuccess) {
//				ImagePtr img = res.payload;
//				cout << img->GetHeight() << "x" << img->GetWidth() << "bbp" << img->GetBitsPerPixel() << endl;
//				stringstream ss;
//				ss << "FLIR-nono8-Spin-" << i << ".pgm";
//				img->Save(ss.str().c_str());
//				cout << "saved image to pgm through spin api" << endl;
//
//				Mat in = FLIRCameraController::ConvertToCVMat(img);
//				stringstream ss2;
//				ss2 << "FLIR-converted-opencv-mono8-" << i << ".pgm";
//				cv::imwrite(ss2.str(), in);
//				cout << "convert and save spiner to cv::mat and save " << endl;
//
//
//				Mat grad_y(ROWS, COLS, in.depth());
//				Mat grad_x(ROWS, COLS, in.depth());
//				plSobel(in, grad_x, grad_y, ROWS, COLS);
//				imwrite("grad_x.jpg", grad_x);
//				imwrite("grad_y.jpg", grad_y);
//
//				cout << "--------------------------------------" << endl;
//
//			} else {
//				cout << "Application Exception FLIR Camera" << endl;
//				cout << res.errorMessage << endl;
//			}
//
//		}
//
//		FLIRCameraController::GetInstance()->Clear();
//		cout << "SUCCESS !!!!!!!!!!!!!!!!!!!!!!!! " << endl;
//	} catch (std::exception & ex) {
//		cout << "Runtime/System Exception FLIR camera" << endl;
//		FLIRCameraController::GetInstance()->Clear();
//	}

	return 0;
}
