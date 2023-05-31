
#include"../myutils.h"
#include "../camera/Camera.h"
#include "../net/net.h"
#include "../ImageServer.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>



#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <chrono>

using namespace cv;

int mythresholding_test(string imageSeed = "data/ori.jpg",int N = 10000 ) {

	unsigned int threshold_value = 128;
	unsigned int max_binary_value = 255;

	Camera* camera0 = Camera::GetCamera(0, 10);
	std::cout << "-------------------------------------------------------------" << std::endl;
	std::cout << "INFO: Runs:" << N << "00 PL/HLS threshold() " << std::endl;
	auto start00 = std::chrono::steady_clock::now();
	for (int i = 0; i < N; i++) {

		Mat in = camera0->GetFrame();
		string name = string("data/cap-") + to_string(i) + ".jpg";
		cout << name << endl;
		imwrite(name, in);
		cout << "Captured Camera Image " << in.rows << ":" << in.cols << endl;

		cvtColor(in, in, COLOR_RGB2GRAY);
		resize(in, in, Size(COLUMNS, ROWS), INTER_LINEAR);
		string name2 = string("data/cap-grey-resized-") + to_string(i) + ".jpg";
		imwrite(name2, in);

		Mat out(ROWS, COLUMNS, CV_8UC1);
		plMyThresh00(in, out);
		string name3 = string("data/cap-grey-resized-pl") + to_string(i)+ ".jpg";
		imwrite(name3, out);

//		Frame aFrame;
//		aFrame.seq = i++;
//		// TODO
//		//aFrame.gpsTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
//		memcpy(aFrame.img, aFrame.img, ROWS * COLUMNS * DEPTH);
//		circularBufferCamera0.push_back(aFrame);
//
//		//udpUtil->Send(host, port, Serialize(aFrame), sizeof(Frame));
//
//		cout << name3 << endl;
	}
	auto end00 = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds_00 = end00 - start00;
	std::cout << "Elapsed Time: " << elapsed_seconds_00.count() << "s\n";
	std::cout << "Time per Unit: " << elapsed_seconds_00.count() / N << "s\n";
	std::cout << "-------------------------------------------------------------" << std::endl;

//	std::cout << "-------------------------------------------------------------" << std::endl;
//	std::cout << "INFO: Runs:" << N << "01 PL/HLS threshold() " << std::endl;
//	auto start01 = std::chrono::steady_clock::now();
//	for (int i = 0; i < N; i++) {
//		plThresh01(in, out);
//	}
//	auto end01 = std::chrono::steady_clock::now();
//	std::chrono::duration<double> elapsed_seconds_01 = end01 - start01;
//	std::cout << "Elapsed Time: " << elapsed_seconds_01.count() << "s\n";
//	std::cout << "Time per Unit: " << elapsed_seconds_01.count() / N << "s\n";
//	std::cout << "-------------------------------------------------------------" << std::endl;
//
//	Mat dst_golden = in.clone();
//	std::cout << "-------------------------------------------------------------" << std::endl;
//	std::cout << "INFO: Run:" << N << " OpenCV threshold()" << std::endl;
//	auto startCV = std::chrono::steady_clock::now();
//	for (int i = 0; i < N; i++) {
//		threshold(in, dst_golden, threshold_value, max_binary_value, THRESH_BINARY);
//	}
//	auto endCV = std::chrono::steady_clock::now();
//	std::chrono::duration<double> elapsed_seconds_cv = endCV - startCV;
//	std::cout << "Elapsed Time: " << elapsed_seconds_cv.count() << "s\n";
//	std::cout << "Time per Unit: " << elapsed_seconds_cv.count() / N << "s\n";
//	std::cout << "-------------------------------------------------------------" << std::endl;

	return 0;
}

