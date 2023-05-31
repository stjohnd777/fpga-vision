#include "xcl2.hpp"
#include "PLRemap.h"


#include <string>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <sstream>
#include <tuple>
#include <time.h>

#include <opencv2/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <boost/circular_buffer.hpp>


#include <CL/cl2.hpp>

#include "PLRemap.h"
#include "common.h"
#include "FLIR/CameraManager.h"
#include "MyImageServer.h"
#include "OpenCvCamera.h"


using namespace std;
using namespace cv;



using namespace lmc;
int numberFrames =10;
bool isCameraGaurd = false;
bool isWriteFile = true;
int usbCameraId = 0;
std::string imgSGS_2592x1944x8x1Path   = "data/sgs_2592x1944x8x1.png";
std::string imgFLIR_2592x1944x16x1Path = "data/flir-2592x1944x16x1.png";



int main(int arc, char** argv) {

	cout << "***********************************************" << endl;
	cout << "REPMAP POC :" << endl;
	cout << "***********************************************" << endl << endl << endl;

	cv::Mat map_x(ROWS, COLUMNS, CV_32FC1);
	cv::Mat map_y(ROWS, COLUMNS, CV_32FC1);
	CreateRemapDefinitionFlipHorz(ROWS, COLUMNS, map_x, map_y);


	int mode = arc >= 2 ? std::stoi(argv[1]) : 0;
	cout << "***********************************************" << endl;
	cout << "Entered Mode :" << mode << endl;
	cout << "***********************************************" << endl << endl << endl;


	if ( mode == 0) {
		// Test USB Camera
		for (auto i = 0; i < numberFrames; i++) {
			OpenCvCamera usbCamera(0);
			auto mat = usbCamera.GetFrame();
			cout << i << "  >>> USB Camera Image Acquired: " << mat.rows << "x" << mat.cols << "bytes per pixel:" << mat.elemSize() << endl;
		    stringstream  ss;
			ss << "usb-" << usbCameraId <<"-" << i << ".png";
			imwrite(ss.str(),mat);
		}

	} else if ( mode == 1) {
		// FLIR
	    int aquiredImages = 0;
		cout << "***********************************************" << endl;
		cout << "MODE FLIR GIGE SPINNAKER TEST :" << endl;
		cout << "***********************************************" << endl << endl << endl;
		try {

			CameraManager::BUFFER_COUNT = 10;
			CameraManager::defaultPixelFormat = PixelFormatEnums::PixelFormat_Mono8;
			CameraManager::LOGGING = false;


			CameraManager::Init(PixelFormatEnums::PixelFormat_Mono8);
			auto cameraCount =  CameraManager::GetCameraCount();
			cout << "Camera Count: " << cameraCount << endl;

			for (auto i = 0; i < numberFrames; i++) {

				for ( auto index = 0 ; index < cameraCount; index++){


					cout << i << ":" << index << " --------------------------------------" << endl;
					GetImageResponse res =  CameraManager::GetImageFromCamera(index,PixelFormatEnums::PixelFormat_Mono8);

					//FLIRCameraController::GetInstance(index)->GetImage(index,10*1000);

					if (res.isSuccess) {
						aquiredImages++;

						ImagePtr img = res.payload;
						cout << ">> capture:" << i <<
								" >> index:" << index <<
								" Image: " << img->GetHeight() << "x" << img->GetWidth() << "bbp" << img->GetBitsPerPixel() << endl;

						stringstream ss;
						ss << i << "-FLIR-MONO8-CAMERA-" << index << ".pgm";
						img->Save(ss.str().c_str());
						cout <<  " Saved Acquired Image to " << ss.str().c_str() <<  endl;

//						Mat in = FLIRCameraController::ConvertToCVMat(img);
//						stringstream ss2;
//						ss2 << i << "-MAT-MONO8-CAMERA-" << index << ".pgm";
//						cv::imwrite(ss2.str(), in);
//						cout << i << " Converted ImagePtr To cv::Mat:" <<  ss2.str() << endl;
//
//						cv::Mat remapOutR = plRemapMono8(in, map_x, map_y);
//						cout << i << " >>> Success plremap mono8 image " << endl;
//						stringstream ss3;
//						ss3 << i<< "-REMAP-MONO8-CAMERA" << index << ".pgm";
//						imwrite(ss3.str(), remapOutR);
//						cout << i << " Result Saved to File: " << ss3.str() << endl;

					} else {
						cout << "capture:" <<  i << "index:" << index <<  " Error .... Application Exception FLIR Camera" << endl;
						cout <<  res.errorMessage << endl;
					}

					cout << i << ":" << index << " --------------------------------------" << endl;

				}

				cout << endl << endl ;
				mysleep(2*1000);

			}
            cout << ">>> We Acquired " << aquiredImages << endl;
			CameraManager::GetInstance()->Clear(0);
			CameraManager::GetInstance()->Clear(1);
			cout << "SUCCESS !!!!!!!!!!!!!!!!!!!!!!!! " << endl;

		} catch (std::exception & ex) {
			cout << "Runtime/System Exception FLIR camera" << endl;

		}

	} else if ( mode == 2) { /* Mon8 and Mono16 Simple Test */

		cout << "***********************************************" << endl;
		cout << "MODE TEST Mono8 1592x1944 accelerated remap :" << endl;
		cout << "***********************************************" << endl << endl << endl;
		RunSimpleRemapMono8FileTest(imgSGS_2592x1944x8x1Path);

	}else if ( mode == 3) { /* Mon8 and Mono16 Simple Test */

		cout << "***********************************************" << endl;
		cout << "MODE TEST Mono16 1592x1944 accelerated remap :" << endl;
		cout << "***********************************************" << endl << endl << endl;
		RunSimpleRemapMono16FileTest(imgFLIR_2592x1944x16x1Path);


	} else if ( mode == 4) { /* Mon8 on Xilinx with Timing */
		// Timing
		RunSimpleRemapMono8FileXilinxTest(imgSGS_2592x1944x8x1Path, 10);
	}else if ( mode == 5) { /* Run The Server */

		cout << "***********************************************" << endl;
		cout << "Processing Camera Feed " << endl;
		cout << "TCP Server running on 8080 " << endl;
		cout << "***********************************************" << endl << endl << endl;

		MyImageServer srv(SERVER_PORT, WFOV);

		thread producer([&] () {
			int i =0;
			while (true) {

				GetImageResponse res = CameraManager::GetInstance()->GetImage(7000);
				ImagePtr img;
				Mat cvmat;
				if ( res.isSuccess){
					cout << "***********************************************" << endl;
					cout << "Acquired Image Form FLIR" << endl;
					img = res.payload;
					cout << img->GetHeight() << "x" << img->GetWidth() << "bbp" << img->GetBitsPerPixel() << endl;
					cvmat = CameraManager::ConvertToCVMat(img);
					cout << "***********************************************" << endl;

				} else {
					cout << "***********************************************" << endl;
					cout << "Application Exception FLIR Camera" << endl;
					cout << res.errorMessage << endl ;
					cout << "***********************************************" << endl;
					continue;
				}

				cout << "cv::Mat:" << cvmat.rows << ":" << cvmat.cols << ":" << cvmat.elemSize() << endl;

				if ( isCameraGaurd) {
					//cvmat = cameraGuard(cvmat);
					cvtColor(cvmat, cvmat, COLOR_RGB2GRAY);
					cv::resize(cvmat, cvmat, Size(COLUMNS, ROWS), cv::INTER_LINEAR);

					if (isWriteFile) {
						stringstream ss;
						ss << "/run/media/mmcblk0p1/data/nodejs/app/public/wfov/wfov-modified-" << i << ".jpg";
						imwrite(ss.str(),cvmat);
						cout << "write file:" << ss.str() << endl;
					}
				}


				if (isWriteFile) {
					stringstream ss;
					ss << "/run/media/mmcblk0p1/data/nodejs/app/public/wfov/wfov-" << i << ".jpg";
					imwrite(ss.str(),cvmat);
					cout << "write file:" << ss.str() << endl;
				}

				cv::Mat remapOut = plRemapMonoR16(cvmat, map_x, map_y);

				cout << "SUCCESS >> PL R16 Called and Frame Processed ( Remap )" << endl;

				srv.push_back(remapOut);

				if (isWriteFile) {
					stringstream ss;
					ss << "/run/media/mmcblk0p1/data/nodejs/app/public/remap/flir.remaped-" << i << ".jpg";
					imwrite(ss.str(),remapOut);
					cout << "write file:" << ss.str() << endl;
				}
				i++;
			}
		});

		srv.Start()->join();
		producer.join();

	} else {

	}

	return 0;
}

