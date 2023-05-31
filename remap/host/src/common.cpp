#include "common.h"
#include "myutils.h"

#include <thread>

using namespace std;
using namespace cv;
/*
 * Using different cameras in default mode
 */
cv::Mat cameraGuard(cv::Mat in) {

	if (in.depth() != 0) {
		cvtColor(in, in, COLOR_RGB2GRAY);
		cout << "Camera Guard: Color convert to grey" << endl;
	}

	if (in.rows != ROWS || in.cols != COLUMNS) {
		cv::resize(in, in, Size(COLUMNS, ROWS), cv::INTER_LINEAR);
		cout << "Camera Guard: resize(" << ROWS << "," << COLUMNS << ")" << endl;
	}

	return in;
}




// TODO: sleep time in properties file
void mysleep(unsigned int ms ) {
	this_thread::sleep_for(chrono::milliseconds(ms));
}

/**
 * This function
 *  - reads in mono8 image file
 *  - initializes the remap X and Y maps
 *  - performs cv::remap and saves img to file
 *  - Initializes the plRemap with our methodology
 *  - performs accelerated xf::remap ans saves imf to file
 *  - perfroms a diff on the two remaps
 */
void RunSimpleRemapMono8FileTest(std::string imagePath  ) {

	cout << "***********************************************" << endl;
	cout << "- Remap Mono8 " << endl;

	cout << "- Read Image File " << imagePath << endl;
	cv::Mat src = cv::imread(imagePath,0); // read image Gray Scale
	if (!src.data) {
		cout << "ERROR: Cannot open image  " << imagePath << endl;
		throw "ERROR: Cannot open image ";
	}
	cout << "- Image Shape " << src.rows << ":" << src.cols << ":" << src.elemSize() << endl;

	cv::Mat map_x(src.rows, src.cols, CV_32FC1);
	cv::Mat map_y(src.rows, src.cols, CV_32FC1);
	CreateRemapDefinitionFlipHorz(src, map_x, map_y);
	cout << "- Create Maps X and Y for remap ... Flip File Vertical Center" << endl;

	cv::Mat ocv_remapped;
	cv::remap(src, ocv_remapped, map_x, map_y, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cout << "- remap with opencv " << endl;
	cv::imwrite("ocv_reference_out8.pgm", ocv_remapped); // OpenCV
	cout << "- saved opencv remap : ocv_reference_out_8.pgm" << endl;

	auto imgPlRemaped = plRemapMono8(src, map_x, map_y);
	cout << "- remap plRemapMono8" << endl;
	cv::imwrite("remap-1944x2592-kernel_out8.pgm", imgPlRemaped); // HLS
	cout << "- save accelerated result image : emap-1944x2592-kernel_out8.pgm" << endl;

	cv::Mat diff;
	cout << "- Computing Difference " << endl;
	diff.create(src.rows, src.cols, src.type());
	cv::absdiff(ocv_remapped, imgPlRemaped, diff);
	cv::imwrite("diff8.pgm", diff);
	cout << "***********************************************" << endl << endl << endl;
}

void RunSimpleRemapMono16FileTest(std::string imagePath  ) {

	cout << "***********************************************" << endl;
	cout << "- Remap Mono16 " << endl;

	cout << "- Read Image File " << imagePath << endl;
	cv::Mat src = cv::imread(imagePath); // read image Gray Scale
	if (!src.data) {
		cout << "ERROR: Cannot open image  " << imagePath << endl;
		throw "ERROR: Cannot open image ";
	}
	cout << "- Image Shape " << src.rows << ":" << src.cols << ":" << src.elemSize() << endl;

	cout << "- reference cv::remap " << endl;
	cv::Mat map_x(src.rows, src.cols, CV_32FC1);
	cv::Mat map_y(src.rows, src.cols, CV_32FC1);
	CreateRemapDefinitionFlipHorz(src, map_x, map_y);

	cv::Mat ocv_remapped;
	cv::remap(src, ocv_remapped, map_x, map_y, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cv::imwrite("ocv_reference_out16.jpg", ocv_remapped); // OpenCV
	cout << "- save image opencv : ocv_reference_out16.pgm" << endl;

    cout << "- Execute Accelerated Remap Mono16" << endl;
	auto remapOut = plRemapMonoL16(src, map_x, map_y);
	cv::imwrite("remap-1944x2592-kernel_out16.pgm", remapOut); // HLS
	cout << "- save file for PL image : remap-1944x2592-kernel_out16.pg" << endl;

	cout << "- Compute Difference " << endl;
	cv::Mat diff;
	diff.create(src.rows, src.cols, src.type());
	cv::absdiff(ocv_remapped, remapOut, diff);
	cv::imwrite("diff16.pgm", diff);
	cout << "***********************************************" << endl << endl << endl;
}


/**elementSize
 * This function
 *  - reads in mono8 image file
 *  - initializes the remap X and Y maps
 *  - performs cv::remap N times with timming
 *  - Initializes with RunSimpleRemapMono8FileXilinxTest method
 *  - performs accelerated xf::remap N times with timing
 *  - perfroms a diff on the two remaps
 */
int RunSimpleRemapMono8FileXilinxTest(std::string imagePath, int N) {

	cout << "***********************************************" << endl;
	cout << "Using Xilinx method to manage access to Xilinx PL" << endl;
	cout << "Read Image File " << imagePath << endl;
	cout << "Gather Time Metrics " << N << " runs " << endl;
	cout << "Flip File Vertical Center" << endl;

	cout << "***********************************************" << endl << endl
			<< endl;

	cv::Mat src = cv::imread(imagePath, 0); // read image Gray Scale
	if (!src.data) {
		cout << "ERROR: Cannot open image " << imagePath << endl;
		return EXIT_FAILURE;
	}
	cout << "***********************************************" << endl;
	cout << "Image Shape " << src.rows << ":" << src.cols << ":" << src.depth()
			<< endl;
	cout << "***********************************************" << endl << endl
			<< endl;

	int rows = src.rows;
	int cols = src.cols;
	std::cout << "Input image height : " << rows << std::endl;
	std::cout << "Input image width  : " << cols << std::endl;
	std::cout << "                return ;Input image type   : " << src.type()
			<< std::endl;

	cv::Mat map_x(src.rows, src.cols, CV_32FC1);
	cv::Mat map_y(src.rows, src.cols, CV_32FC1);
	cv::Mat diff(src.rows, src.cols, src.type());
	cv::Mat ocv_remapped(src.rows, src.cols, src.type());
	cv::Mat hls_remapped(src.rows, src.cols, src.type());

	// example map generation, flips the image horizontally
	CreateRemapDefinitionFlipHorz(src, map_x, map_y);

	// OpenCV
	std::cout << "-------------------------------------------------------------"
			<< std::endl;
	std::cout << "INFO: Run OpenCV remap()" << std::endl;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < N; i++) {
		cv::remap(src, ocv_remapped, map_x, map_y, cv::INTER_NEAREST,
				cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
	std::cout << "-------------------------------------------------------------"
			<< std::endl;
	// End OPENCV

	// OpenCL Section:
	size_t image_in_size_bytes = src.rows * src.cols * sizeof(unsigned char)
			* CHANNELS;
	size_t map_in_size_bytes = src.rows * src.cols * sizeof(float);
	size_t image_out_size_bytes = image_in_size_bytes;

	std::cout << "INFO: Running OpenCL section." << std::endl;
	// Get the device:
	cl_int err;
	std::vector<cl::Device> devices = xcl::get_xil_devices();
	cl::Device device = devices[0];
	std::string device_name = device.getInfo < CL_DEVICE_NAME > (&err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " getInfo " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: Device found - " << device_name << std::endl;

	// Context, command queue and device name:
	std::cout << "INFO: OpenCL Context." << std::endl;
	cl::Context context(device, NULL, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " getInfo " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: Context Created on - " << std::endl;

	std::cout << "INFO: OpenCL CommandQueue." << std::endl;
	cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " CommandQueue " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: Command Queue Created on - " << device_name
			<< std::endl;

	// Load binary:Create a kernel:
	std::string kernel_name = "krnl_remap";
	std::string binaryFile = xcl::find_binary_file(device_name, kernel_name);
	cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
	devices.resize(1);
	std::cout << "INFO: Kernel krnl_remap and cl::Program::Binaries  "
			<< std::endl;

	cl::Program program(context, devices, bins, NULL, &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Program " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: cl::Program  " << std::endl;

	cl::Kernel kernel(program, "remap_accel", &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Kernel " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: Created Kernel  " << kernel_name << std::endl;

	// Allocate the buffers:
	cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes,
	NULL, &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Buffer " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: cl::Buffer  " << std::endl;

	cl::Buffer buffer_inMapX(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL,
			&err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Buffer " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: cl::Buffer  " << std::endl;

	cl::Buffer buffer_inMapY(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL,
			&err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Buffer " << err << endl;
		exit(EXIT_FAILURE);
	}

	cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes,
	NULL, &err);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " Buffer " << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: cl::Buffer  " << std::endl;

	std::cout << "INFO: Allocated All Buffers  " << std::endl;

	// Set kernel arguments:
	err = kernel.setArg(0, buffer_inImage);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 0" << err << endl;
		exit(EXIT_FAILURE);
	}
	err = kernel.setArg(1, buffer_inMapX);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 1" << err << endl;
		exit(EXIT_FAILURE);
	}
	err = kernel.setArg(2, buffer_inMapY);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 2" << err << endl;
		exit(EXIT_FAILURE);
	}
	err = kernel.setArg(3, buffer_outImage);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 3" << err << endl;
		exit(EXIT_FAILURE);
	}
	err = kernel.setArg(4, rows);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 4" << err << endl;
		exit(EXIT_FAILURE);
	}
	err = kernel.setArg(5, cols);
	if (err != CL_SUCCESS) {
		cout << __FILE__ << ":" << __LINE__ << " setArg 5" << err << endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "INFO: Set Kernel Arguments  " << std::endl;

	// Initialize the buffers:
	cl::Event event;
	std::cout << "-------------------------------------------------------------" << std::endl;
	std::cout << "INFO: Run HLS remap()" << std::endl;
	auto start2 = std::chrono::steady_clock::now();
	for (int i = 0; i < N; i++) {

		// buffer on the FPGA ,blocking call,buffer offset in bytes ,Size in bytes,Pointer to the data to copy
		size_t offset = 0;
		size_t size = image_in_size_bytes;
		err = queue.enqueueWriteBuffer(buffer_inImage, CL_TRUE, offset, size,
				src.data, nullptr, &event);
		if (err != CL_SUCCESS) {
			cout << __FILE__ << ":" << __LINE__ << " enqueueWriteBuffer(buffer_inImage," << err << endl;
			exit(EXIT_FAILURE);
		}

		err = queue.enqueueWriteBuffer(buffer_inMapX, CL_TRUE, 0,
				map_in_size_bytes, map_x.data, nullptr, &event);
		if (err != CL_SUCCESS) {
			cout << __FILE__ << ":" << __LINE__ << " enqueueWriteBuffer(buffer_inMapX," << err << endl;
			exit(EXIT_FAILURE);
		}

		err = queue.enqueueWriteBuffer(buffer_inMapY, CL_TRUE, 0,
				map_in_size_bytes, map_y.data, nullptr, &event);
		if (err != CL_SUCCESS) {
			cout << __FILE__ << ":" << __LINE__ << " enqueueWriteBuffer(buffer_inMapY," << err << endl;
			exit(EXIT_FAILURE);
		}

		std::cout << "----" << std::endl;
		std::cout << "INFO: Run PLRemap Kernel " << std::endl;

		auto startKern = std::chrono::steady_clock::now();
		// Execute the kernel:
		err = queue.enqueueTask(kernel);
		if (err != CL_SUCCESS) {
			cout << __FILE__ << ":" << __LINE__ << " enqueueTask," << err << endl;
			exit(EXIT_FAILURE);
		}
		queue.finish();

		auto endKern = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds_kern = endKern - startKern;
		std::cout << "Run Time elapsed time: " << elapsed_seconds_kern.count() << "s\n";
		std::cout << "----" << std::endl;

		// Copy Result from Device Global Memory to Host Local Memory
		err = queue.enqueueReadBuffer(buffer_outImage, CL_TRUE, 0,
				image_out_size_bytes, hls_remapped.data, nullptr, &event);
		if (err != CL_SUCCESS) {
			cout << __FILE__ << ":" << __LINE__ << " enqueueReadBuffer(buffer_outImage," << err << endl;
			exit(EXIT_FAILURE);
		}

	}
	auto end2 = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds2 = end2 - start2;
	std::cout << "elapsed time: " << elapsed_seconds2.count() << "s\n";
	std::cout << "-------------------------------------------------------------" << std::endl;

	// Save the results:
	cv::imwrite("ocv_reference_out.jpg", ocv_remapped); // OpenCV
	cv::imwrite("kernel_out.jpg", hls_remapped); // HLS

	// Results verification:
	cv::absdiff(ocv_remapped, hls_remapped, diff);
	cv::imwrite("diff.pgm", diff);

	return 0;
}
