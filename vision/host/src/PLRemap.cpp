#include "PLRemap.h"

#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>

cv::Mat plRemap(cv::Mat& in, cv::Mat& map_x, cv::Mat& map_y){

	static string xclbin = XCLBIN::REMAP;
	static string kernelName = KERNEL_NAME::REMAP;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

	// TODO
//	if ( in.rows != ROWS || in.cols != COLS || in.depth != DEPTH ){
//		throw "Invalid Dimensions on Image ";
//	}

	size_t image_in_size_bytes = in.rows * in.cols * sizeof(unsigned char);
	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);
	size_t image_out_size_bytes = image_in_size_bytes;

	cv::Mat hls_remapped ( in.rows, in.cols, in.type());

	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapX(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inMapY(context, CL_MEM_READ_ONLY, map_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

    // Set kernel arguments:
    int rows = in.rows;
    int cols = in.cols;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inMapX));
    OCL_CHECK(err, err = kernel.setArg(2, buffer_inMapY));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));
    OCL_CHECK(err, err = kernel.setArg(4, rows));
    OCL_CHECK(err, err = kernel.setArg(5, cols));

    cl::Event event;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));

	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapX,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_x.data,        // Pointer to the data to copy
									   nullptr, &event));

	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inMapY,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   map_in_size_bytes, // Size in bytes
									   map_y.data,        // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

	// Copy Result from Device Global Memory to Host Local Memory
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							hls_remapped.data, // Data will be stored here
							nullptr, &event);

	// Clean up:
	queue.finish();

	return hls_remapped;
}



