#include "PLBilateralFilter.h"

#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>

cv::Mat plBilateralFilter(
		cv::Mat& in, float sigma_color, float sigma_space
		)
{

	static string xclbin = XCLBIN::BILATERAL;
	static string kernelName = KERNEL_NAME::BILATERAL;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);


	size_t image_in_size_bytes = in.rows * in.cols * sizeof(unsigned char);
	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);
	size_t image_out_size_bytes = image_in_size_bytes;

	cv::Mat hls_bilateral ( in.rows, in.cols, in.type());

	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

    // Set kernel arguments:
    int rows = in.rows;
    int cols = in.cols;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, sigma_color));
    OCL_CHECK(err, err = kernel.setArg(2, sigma_space));
    OCL_CHECK(err, err = kernel.setArg(3, in.rows));
    OCL_CHECK(err, err = kernel.setArg(4, in.cols));
    OCL_CHECK(err, err = kernel.setArg(5, buffer_outImage));

    cl::Event event;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

	// Copy Result from Device Global Memory to Host Local Memory
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							hls_bilateral.data, // Data will be stored here
							nullptr, &event);

	// Clean up:
	queue.finish();

	return hls_bilateral;
}



