#include "PLNUC.h"

#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>

cv::Mat plNUC(cv::Mat& I, cv::Mat& M, cv::Mat& C){

	static string xclbin = XCLBIN::NUC;
	static string kernelName = KERNEL_NAME::NUC;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);


	size_t nBytes = I.rows * I.cols * sizeof(unsigned char);


	cv::Mat nuc ( I.rows, I.cols, I.type());

	cl_int err;
    OCL_CHECK(err, cl::Buffer inBuffer_I(context, CL_MEM_READ_ONLY, nBytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_M(context, CL_MEM_READ_ONLY, nBytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_C(context, CL_MEM_READ_ONLY, nBytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer outBuffer_NUC(context, CL_MEM_WRITE_ONLY, nBytes, NULL, &err));

    // Set kernel arguments:
    unsigned char rows = I.rows;
    unsigned char cols = I.cols;
    unsigned arg =0;
    OCL_CHECK(err, err = kernel.setArg(arg++, inBuffer_I));
    OCL_CHECK(err, err = kernel.setArg(arg++, buffer_M));
    OCL_CHECK(err, err = kernel.setArg(arg++, buffer_C));
    OCL_CHECK(err, err = kernel.setArg(arg++, rows));
    OCL_CHECK(err, err = kernel.setArg(arg++, cols));
    OCL_CHECK(err, err = kernel.setArg(arg++, outBuffer_NUC));


    cl::Event event;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(inBuffer_I,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   nBytes, // Size I bytes
									   I.data,            // Pointer to the data to copy
									   nullptr, &event));

	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_M,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   nBytes, // Size I bytes
									   M.data,        // Pointer to the data to copy
									   nullptr, &event));

	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_C,     // buffer on the FPGA
									   CL_TRUE,           // blocking call
									   0,                 // buffer offset in bytes
									   nBytes, // Size I bytes
									   C.data,        // Pointer to the data to copy
									   nullptr, &event));


	// Execute the kernel:
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

	// Copy Result from Device Global Memory to Host Local Memory
	queue.enqueueReadBuffer(outBuffer_NUC, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							nBytes,
							nuc.data, // Data will be stored here
							nullptr, &event);

	// Clean up:
	queue.finish();

	return nuc;
}



