#include "PLMedian.h"

#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>


cv::Mat plMedian3x3(cv::Mat& in){

	static string xclbin = XCLBIN::MEDIAN;
	static string kernelName = KERNEL_NAME::MEDIAN;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

	cout << "plMedian3x3" << endl;

	size_t image_in_size_bytes = in.rows * in.cols * sizeof(unsigned char);
	size_t map_in_size_bytes 	= in.rows * in.cols * sizeof(float);
	size_t image_out_size_bytes = image_in_size_bytes;
	cv::Mat out ( in.rows, in.cols, in.type());

	cl_int err;
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

	cout << "plMedian3x3: created buffers" << endl;

    int rows = in.rows;
    int cols = in.cols;
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, rows));
    OCL_CHECK(err, err = kernel.setArg(2, cols));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));


    cout << "plMedian3x3: set kernel arguments" << endl;

    cl::Event event;
	OCL_CHECK(err,
			  queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
									   CL_TRUE,             // blocking call
									   0,                   // buffer offset in bytes
									   image_in_size_bytes, // Size in bytes
									   in.data,            // Pointer to the data to copy
									   nullptr, &event));


	cout << "plMedian3x3: enqueue input buffer" << endl;

	// Execute the kernel:
	OCL_CHECK(err, err = queue.enqueueTask(kernel));

	cout << "plMedian3x3: enqueued median kernel should be running " << endl;


	// Copy Result from Device Global Memory to Host Local Memory
	queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
							CL_TRUE,         // blocking call
							0,               // offset
							image_out_size_bytes,
							out.data, // Data will be stored here
							nullptr,
							&event);

	cout << "plMedian3x3: read the fpga read out buffer" << endl;

	// Clean up:
	queue.finish();

	cout << "plMedian3x3: finish" << endl;

    return out;
}
