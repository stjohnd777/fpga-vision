#include "PLRemap.h"
#include "myutils.h"
#include "common.h"

#include <string>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <sstream>
#include <tuple>
#include <time.h>
#include <chrono>
#include <vector>

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
#include "BlkFlyCameraManager.h"
#include "PLRemap.h"
#include "common.h"
#include "MyImageServer.h"
#include "OpenCvCamera.h"


#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;


using namespace std;
using namespace cv;

int main(int arc, char** argv) {

	std: vector<thread*> vThreads;

	BlkFlyCameraManager::Init(Configs::pixelFmt, Configs::numBuffers);
	unsigned int cameraCount = BlkFlyCameraManager::GetCameraCount();
	cout << "Camera Count: " << cameraCount << endl;

	for (unsigned int cameraIndex = 0; cameraIndex < cameraCount; cameraIndex++) {

		int port = SERVER_PORT + cameraIndex;
		cout << "***********************************************" << endl;
		cout << "Camera Indexed at " << cameraIndex << " with TCP Server running on " << port << endl;
		cout << "***********************************************" << endl << endl;
		MyImageServer srv(port, cameraIndex);

		thread* imgProducerThread = new thread(
			[&] (unsigned int cameraIndex) {
				int i =0;
				while (true) {

					GetImageResponse res = BlkFlyCameraManager::GetImageFromCamera(cameraIndex);
					ImagePtr img;
					Mat cvmat;

					if ( res.isSuccess) {
						cout << "***********************************************" << endl;
						cout << "Acquired Image Form FLIR Camera at index " << cameraIndex <<  endl;
						img = res.payload;
						cout << img->GetHeight() << "x" << img->GetWidth() << "@bbp:" << img->GetBitsPerPixel() << endl;
						cvmat = BlkFlyCameraManager::ConvertToCVMat(img);
						cout << "***********************************************" << endl;
						break;

					} else {
						cout << "***********************************************" << endl;
						cout << "No Image Acquired Yet Form FLIR Camera at index " << cameraIndex  << endl;
						cout << res.errorMessage << endl;
						cout << "***********************************************" << endl;
						continue;
					}

					if (Configs::isWriteFile) {
						stringstream ss;
						ss << Configs::imgDir << "/wfov/wfov-" << i%10 << Configs::imgfmt;
						imwrite(ss.str(),cvmat);
						cout << "write file:" << ss.str() << endl;
					}

					if ( Configs::isImageRaw) {
						srv.push_back(cvmat);
					}

					if (Configs::isRemap) {
						cv::Mat map_x(ROWS, COLUMNS, CV_32FC1);
						cv::Mat map_y(ROWS, COLUMNS, CV_32FC1);
						CreateRemapDefinitionFlipHorz(ROWS, COLUMNS, map_x, map_y);

						cv::Mat remapOut = plRemapMonoR16(cvmat, map_x, map_y);
						cout << "SUCCESS >> PL R16 Called" << endl;
						srv.push_back(remapOut);
						if (Configs::isWriteFile) {
							stringstream ss;
							ss << Configs::imgDir << "/remap/remap-" << i << Configs::imgfmt;
							imwrite(ss.str(),remapOut);
							cout << "write file:" << ss.str() << endl;
						}
					}

					i++;
				}
			}, cameraIndex);

		srv.Start();
		vThreads.push_back(imgProducerThread);
	}

	if ( cameraCount >= 2){

		thread* t = new thread([&](){
			MyImageServer srv(9000, -1);
			srv.Start();
			while (true) {
				GetImageResponse* res = SteroPair(0, 1);
				ImagePtr l = res[0].payload;
				srv.push_back(BlkFlyCameraManager::ConvertToCVMat(l));
				ImagePtr r = res[1].payload;
				srv.push_back(BlkFlyCameraManager::ConvertToCVMat(4));
			}
			});

		vThreads.push_back(t);
	}

	for (auto t : vThreads) {
		t->join();
	}

	return 0;
}

