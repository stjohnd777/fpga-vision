#include "all.h"
#include <boost/asio.hpp>
#include <stdlib.h>

void FlipHorz(cv::Mat& src, cv::Mat& map_x, cv::Mat& map_y) {
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			float valx = (float) (src.cols - j - 1);
			float valy = (float) i;
			map_x.at<float>(i, j) = valx;
			map_y.at<float>(i, j) = valy;
		}
	}
}


std::tuple<cv::Mat,cv::Mat> GetRemapXYMapsFromFiles(int rows, int cols,char* mapx, char* mapy){
	cv::Mat map_x, map_y;
    map_x.create(rows, cols, CV_32FC1);
    map_y.create(rows, cols, CV_32FC1);
    FILE *fp_mx, *fp_my;
    fp_mx = fopen(mapx, "r");
    fp_my = fopen(mapy, "r");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float valx, valy;
            if (fscanf(fp_mx, "%f", &valx) != 1) {
                fprintf(stderr, "Not enough data in the provided map_x file ... !!!\n ");
            }
            if (fscanf(fp_my, "%f", &valy) != 1) {
                fprintf(stderr, "Not enough data in the provided map_y file ... !!!\n ");
            }
            map_x.at<float>(i, j) = valx;
            map_y.at<float>(i, j) = valy;
        }
    }
    return std::make_tuple(map_x,map_y);
}


void readTextFile(string path) {
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
}

void splitString(std::vector<std::string> &result, std::string str,
		char delim) {
	std::string tmp;
	std::string::iterator i;
	result.clear();

	for (i = str.begin(); i <= str.end(); ++i) {
		if ((const char) *i != delim && i != str.end()) {
			tmp += *i;
		} else {
			if (!tmp.empty()) {
				result.push_back(tmp);
			}
			tmp = "";
		}
	}
}



std::vector<std::string> mysplit(const std::string &str,
		const std::string &delim) {
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

using boost::asio::ip::tcp;
thread *MyImageServer(int cameraId = 0, int port = 8080, int qNumber = 0) {

	thread *ptrThread =
			new thread(
					[&](int cameraId, int port) {
						try {
							boost::asio::io_context io_context;
							tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
							while (true) {

								// Wait for a client to connect
								tcp::socket socket(io_context);
								acceptor.accept(socket);
								std::cout << "Client Connected" << std::endl;

								// GET REQUEST
								boost::asio::streambuf request_buf;
								boost::asio::read_until(socket, request_buf, "\n");
								std::string request = boost::asio::buffer_cast<const char *>(request_buf.data());
								auto tokens = mysplit(request, "|");
								std::cout << "Client Sent Data " << request << std::endl;

								// cameraId, time qNumber
								Frame aFrame;
								if ( cameraId == 0) {
									aFrame = LookUpCameraFrame(0,0,0);
								} else {
									aFrame = LookUpSGSFrame(0);
								}
								boost::asio::write(socket, boost::asio::buffer(aFrame.img, aFrame.len));

								std::cout << "Image Bytes:" << aFrame.len << " Sent To Client " << std::endl;
							}
						}
						catch (std::exception &e) {
							std::cerr << "Exception: " << e.what() << std::endl;
							return;

						}
					}, cameraId, port);

	return ptrThread;
}


void mysleep(unsigned int ms = 1000){
	this_thread::sleep_for(chrono::milliseconds(ms));
}

// example
std::mutex remap_mtx;
cv::Mat remap(cv::Mat in){
	cv::Mat map_x(in.rows, in.cols, CV_32FC1);
	cv::Mat map_y(in.rows, in.cols, CV_32FC1);
	FlipHorz(in, map_x, map_y);
	std::lock_guard<std::mutex> locker(remap_mtx);
	cv::Mat remapOut = plRemap(in, map_x, map_y);
	return remapOut;
}



bool isCam = true;
bool isSGS = false;


string node_base = "data/nodejs/app/simple/public";

int main(int arc, char** argv) {

	cout << "Enter main() " << endl;

	//string pathtoimg = "data/sgs_image_0.png";
	string pathtoimg = "data/stat4k.jpg";
	auto in = imread(pathtoimg, 0);
	if (!in.data) {
		cout << "Failed to load the image ... !!!\n " << pathtoimg << endl;
		return -1;
	}

	//cvtColor(in, in, COLOR_RGB2GRAY);
	cv::resize(in, in, Size(COLUMNS, ROWS), cv::INTER_LINEAR);
	cout << "Grey and Resized to 1920x1080   " << endl;

	int rows = in.rows;
	int cols = in.cols;
	cout << "loaded image" << endl;

	cout << "--------------------------" << endl;
	cv::Mat map_x(rows, cols, CV_32FC1);
	cv::Mat map_y(rows, cols, CV_32FC1);
	FlipHorz(in, map_x, map_y);
	auto remapOut = plRemap(in, map_x, map_y);
	imwrite("Remap.jpg", remapOut);
	cout << "remap() done" << endl;

	cout << "---------------------------" << endl;
	unsigned char maxval = 256;
	unsigned char thresh = 100;
	Mat mythreshOut00(rows, cols, in.depth());
	plMyThresh00(in, mythreshOut00, thresh, maxval);
	imwrite("MyThresh.jpg", mythreshOut00);
	cout << "plMyThresh00() done" << endl;

	cout << "---------------------------" << endl;
	Mat mythreshOut01(rows, cols, in.depth());
	plMyThresh01(in, mythreshOut01, thresh, maxval);
	imwrite("MyThresh.jpg", mythreshOut01);
	cout << "plMyThresh01() done" << endl;

	cout << "---------------------------" << endl;
	Mat threshOut(rows, cols, in.depth());
	plThreshold(in, threshOut, 128, 255);
	imwrite("Thresh.jpg", threshOut);
	cout << "xilinx plThreshold() done" << endl;

	cout << "---------------------------" << endl;
	float sigma = 0.5f;
	auto gaussOut = plGaussian(in, sigma);
	imwrite("Gaussian.jpg", gaussOut);
	cout << "plGaussian() done" << endl;

	cout << "---------------------------" << endl;
	unsigned char threshold = 20;
	Mat fastOut = plFast(in, threshold, rows, cols);
	imwrite("Fast.jpg", fastOut);
	cout << "plFast() done" << endl;

	cout << "---------------------------" << endl;
	Mat grad_y(rows, cols, in.depth());
	Mat grad_x(rows, cols, in.depth());
	plSobel(in, grad_x, grad_y, rows, cols);
	imwrite("grad_x.jpg", grad_x);
	imwrite("grad_y.jpg", grad_y);
	cout << "plSobel() done" << endl;

	cout << "---------------------------" << endl;
	Mat median = plMedian3x3(in);
	imwrite("median.jpg", median);
	cout << "plMedin3x3() done" << endl;

	cout << "---------------------------" << endl;
	float sigma_color = 128; //rng.uniform(0.0, 1.0) * 255;
	float sigma_space = .5; //rng.uniform(0.0, 1.0);
	Mat bi = plBilateralFilter(in, sigma_color, sigma_space);
	imwrite("plBilateralFilter.jpg", bi);
	imwrite("grad_y.jpg", grad_y);
	cout << "plBilateralFilter() done" << endl;

	cv::VideoCapture * pCap = 0;
	Camera* camera0 = 0;
	try {
		cout << "---------------------------" << endl;
		cout << "Camera Thread Starting  " << endl;
		camera0 = Camera::GetCamera(0, 10);
		/*
    	 pCap = camera0->getVideoCapture();
		 pCap->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
		 pCap->set(cv::CAP_PROP_FRAME_HEIGHT,1080);
		 */
		cout << "Acquired Camera  " << 0 << endl;
	} catch (...) {
		cout << "Cound Not Acquired Camera  " << 0 << endl;
		return -1;
	}

	// A Proposed precedence is INBOX, SGS_REQUEST, FILE_REQUEST, CAMERA
	enum IMG_SRC {
		CAMERA, SGS, FILE, INBOX
	};

	//const string INBOX = "data/nodejs/app/simple/public/inbox";

	auto getImageToProcess = [&]( int mode){
		Mat in;

		if ( mode == 0) {
		     in = camera0->GetFrame();
			cout << "Camera acquired frame   " << in.rows << ":" << in.cols << ":" << in.depth() << endl;
			return in;
		}
		if ( mode == 1){
			string path = "data/sgs_image_0.png";
			in = imread(path, 1);
		}
		if ( mode == 2) {
			string path = "data/stat4k.jpg";
			in = imread(path, 1);
		}
		return in;

	};


	thread cameraProcessingThread(
			[&]() {

				for (int i =0; i < 600; i++) {

					Mat in = getImageToProcess(i%3);
//					camera0->GetFrame();
//					cout << "Camera acquired frame   " << in.rows << ":" << in.cols << ":" << in.depth() << endl;

					// RAW 0
					// TODO: Get the actual camera
					cvtColor(in, in, COLOR_RGB2GRAY);
					cv::resize(in, in, Size(COLUMNS, ROWS), cv::INTER_LINEAR);
					cout << "Grey and Resized to 1920x1080   " << endl;
					QueueCameraFrame(Frame(0,in));
					// End Raw

					// Remap 1
					cv::Mat map_x(in.rows, in.cols, CV_32FC1);
					cv::Mat map_y(in.rows, in.cols, CV_32FC1);
					FlipHorz(in, map_x, map_y);
					cv::Mat remapOut = plRemap(in, map_x, map_y);
					cout << "PL Called and Frame Processed ( Remap )" << endl;
					QueueCameraFrame(Frame(1,remapOut));
					// save to file
					stringstream ssRemp;
					ssRemp << node_base << "/remap/remap-" << i << ".jpg";
					imwrite(ssRemp.str(),remapOut);
					cout << ssRemp.str() << "written!";
					// End Remap

					// Thresholding 2
					Mat threshOut(in.rows, in.cols, in.depth());
					plThreshold(in, threshOut, 128, 255);
					QueueCameraFrame(Frame(2,threshOut));
					stringstream ssThresh;
					ssThresh << node_base << "/thresholding/thresh-" << i << ".jpg";
					imwrite(ssThresh.str(), threshOut);// save to file
					// END Thresholding

					// Median 3
					Mat median = plMedian3x3(in);
					QueueCameraFrame(Frame(3,median));
					stringstream ssMedian;
					ssMedian << node_base << "/median/median-" << i << ".jpg";
					imwrite(ssMedian.str(), median);// save to file
					// END Median

					// FAST : 3
					Mat fastOut = plFast(in, 20, in.rows, in.cols);
					QueueCameraFrame(Frame(4,fastOut));
					stringstream ssFast;
					ssFast << node_base << "/fast/fast-" << i << ".jpg";
					imwrite(ssFast.str(), fastOut);
					cout << "plFast() done" << endl;
					// END FAST

					// SOBEL XY : 5
					Mat grad_y(in.rows, in.cols, in.depth());
					Mat grad_x(in.rows, in.cols, in.depth());
					plSobel(in, grad_x, grad_y, rows, cols);
					QueueCameraFrame(Frame(5,grad_x));
					QueueCameraFrame(Frame(5,grad_y));
					stringstream ssSobelX;
					ssSobelX << node_base <<  "/sobel/gradx-" << i << ".jpg";
					imwrite(ssSobelX.str(), grad_x);
					stringstream ssSobelY;
					ssSobelY << node_base << "/sobel/grady-" << i << ".jpg";
					imwrite(ssSobelY.str(), grad_y);
					cout << "plSobel() done" << endl;
					// END SOBEL

					// Gaussian : 6
					float sigma = 0.5f;
					auto gaussOut = plGaussian(in, sigma);
					QueueCameraFrame(Frame(6,gaussOut));
					stringstream ssGaussian;
					ssGaussian << node_base << "/gaussian/gaussian-" << i << ".jpg";
					imwrite(ssGaussian.str(), gaussOut);
					cout << "plGaussian() done" << endl;

					// plBilateralFilter : 7
					float sigma_color = 128; //rng.uniform(0.0, 1.0) * 255;
					float sigma_space = .5; //rng.uniform(0.0, 1.0);
					Mat bi = plBilateralFilter(in, sigma_color, sigma_space);
					QueueCameraFrame(Frame(7,bi));
					stringstream ssBilateral;
					ssBilateral << node_base << "/bilateral/bilateral-" << i << ".jpg";
					imwrite(ssBilateral.str(), bi);
					cout << "plBilateralFilter() done" << endl;

					cout << "---------------------------" << endl;

					//sleep();
				}
			});

	int cameraId = 0;
	int port = 8080;
	auto ptrThreadCameraService = MyImageServer(cameraId, port);


	ptrThreadCameraService->join();


	cameraProcessingThread.join();


	cout << "end" << endl;

	return 0;
}
