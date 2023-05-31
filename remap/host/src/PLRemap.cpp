#include "PLRemap.h"

#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>

cv::Mat plRemapMono8(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y){

	static string xclbin ="binary_container_1.xclbin";
	static string kernelName = "remap_accel";
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

    int rows = in.rows;
    int cols = in.cols;
	size_t image_in_size_bytes = rows * cols * sizeof(unsigned char) * 1 * 1;
	cout << "Image Source:" << rows << ":" << cols << ": bytes "  << image_in_size_bytes << endl;
	size_t image_out_size_bytes = image_in_size_bytes;

	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);


	cv::Mat hls_remapped ( in.rows, in.cols, in.type());

	cout << "Allocate Buffers " << endl;
	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapX(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapY(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));
    cout << "Allocated Buffers " << endl;

    // Set kernel arguments:
    cout << "Set Kernel Arguments " << endl;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inMapX));
    OCL_CHECK(err, err = kernel.setArg(2, buffer_inMapY));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));
    OCL_CHECK(err, err = kernel.setArg(4, rows));
    OCL_CHECK(err, err = kernel.setArg(5, cols));
    cout << "Success Kernel Arguments " << endl;

    cl::Event event;
    cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));

	 cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapX,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_x.data,        // Pointer to the data to copy
									   nullptr, &event));

	cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapY,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_y.data,        // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
    cout << "Execute Kernel  " << endl;
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

    cout << "Wait Kernel Arguments " << endl;

    queue.finish();

    // Copy Result from Device Global Memory to Host Local Memory
    cout << "Read out Kernel Results   " << endl;
    cout << "enqueueReadBuffer " << endl;
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							hls_remapped.data, // Data will be stored here
							nullptr, &event);

	cout << "Done Reading out Kernel Results   " << endl;


	return hls_remapped;
}


cv::Mat plRemapMonoL16(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y){

	static string xclbin ="binary_container_1.xclbin";
	static string kernelName = "remap_accel_L16";
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

    int rows = in.rows;
    int cols = in.cols;
	size_t image_in_size_bytes = rows * cols * sizeof(unsigned char) * 2;
	cout << "Image Source:" << rows << ":" << cols << ": bytes "  << image_in_size_bytes << endl;
	size_t image_out_size_bytes = image_in_size_bytes;

	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);


	cv::Mat hls_remapped ( in.rows, in.cols, in.type());

	cout << "Allocate Buffers " << endl;
	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapX(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapY(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));
    cout << "Allocated Buffers " << endl;

    // Set kernel arguments:
    cout << "Set Kernel Arguments " << endl;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inMapX));
    OCL_CHECK(err, err = kernel.setArg(2, buffer_inMapY));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));
    OCL_CHECK(err, err = kernel.setArg(4, rows));
    OCL_CHECK(err, err = kernel.setArg(5, cols));
    cout << "Success Kernel Arguments " << endl;

    cl::Event event;
    cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));

	 cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapX,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_x.data,        // Pointer to the data to copy
									   nullptr, &event));

	cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapY,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_y.data,        // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
    cout << "Execute Kernel  " << endl;
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

    cout << "Wait Kernel Arguments " << endl;

    queue.finish();

    // Copy Result from Device Global Memory to Host Local Memory
    cout << "Read out Kernel Results   " << endl;
    cout << "enqueueReadBuffer " << endl;
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							hls_remapped.data, // Data will be stored here
							nullptr, &event);

	cout << "Done Reading out Kernel Results   " << endl;


	return hls_remapped;
}

cv::Mat plRemapMonoR16(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y){

	static string xclbin ="binary_container_1.xclbin";
	static string kernelName = "remap_accel_R16";
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

    int rows = in.rows;
    int cols = in.cols;
	size_t image_in_size_bytes = rows * cols * sizeof(unsigned char) * 2;
	cout << "Image Source:" << rows << ":" << cols << ": bytes "  << image_in_size_bytes << endl;
	size_t image_out_size_bytes = image_in_size_bytes;

	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);


	cv::Mat hls_remapped ( in.rows, in.cols, CV_16UC1);

	cout << "Allocate Buffers " << endl;
	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapX(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapY(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));
    cout << "Allocated Buffers " << endl;

    // Set kernel arguments:
    cout << "Set Kernel Arguments " << endl;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inMapX));
    OCL_CHECK(err, err = kernel.setArg(2, buffer_inMapY));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));
    OCL_CHECK(err, err = kernel.setArg(4, rows));
    OCL_CHECK(err, err = kernel.setArg(5, cols));
    cout << "Success Kernel Arguments " << endl;

    cl::Event event;
    cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));

	 cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapX,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_x.data,        // Pointer to the data to copy
									   nullptr, &event));

	cout << "enqueueWriteBuffer " << endl;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapY,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_y.data,        // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
    cout << "Execute Kernel  " << endl;
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

    cout << "Wait Kernel Arguments " << endl;

    queue.finish();

    // Copy Result from Device Global Memory to Host Local Memory
    cout << "Read out Kernel Results   " << endl;
    cout << "enqueueReadBuffer " << endl;
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							hls_remapped.data, // Data will be stored here
							nullptr, &event);

	cout << "Done Reading out Kernel Results   " << endl;


	return hls_remapped;
}







// example map generation, flips the image horizontally
void CreateRemapDefinitionFlipHorz(cv::Mat& src, cv::Mat& map_x,
		cv::Mat& map_y) {
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			float valx = (float) (src.cols - j - 1);
			float valy = (float) i;
			map_x.at<float>(i, j) = valx;
			map_y.at<float>(i, j) = valy;
		}
	}
}

void CreateRemapDefinitionFlipHorz(int rows, int cols, cv::Mat& map_x,
		cv::Mat& map_y) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			float valx = (float) (cols - j - 1);
			float valy = (float) i;
			map_x.at<float>(i, j) = valx;
			map_y.at<float>(i, j) = valy;
		}
	}
}

// TODO : properties file for location mapX and mapY files
std::tuple<cv::Mat, cv::Mat> GetRemapXYMapsFromFiles(int rows, int cols, char* mapx , char* mapy ) {
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
				fprintf(stderr,
						"Not enough data in the provided map_x file ... !!!\n ");
			}
			if (fscanf(fp_my, "%f", &valy) != 1) {
				fprintf(stderr,
						"Not enough data in the provided map_y file ... !!!\n ");
			}
			map_x.at<float>(i, j) = valx;
			map_y.at<float>(i, j) = valy;
		}
	}
	return std::make_tuple(map_x, map_y);
}


