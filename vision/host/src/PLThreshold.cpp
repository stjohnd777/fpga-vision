#include "PLThreshold.h"

#include "algo.h"
#include "myutils.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <stdlib.h>

using namespace std;
using namespace cv;

void plThreshold(cv::Mat& in, cv::Mat& out, unsigned char thresh , unsigned char maxval ) {

	static string xclbin = "binary_container_1.xclbin";
	static string kernelName = "threshold_accel";
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue commandQueue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel krnl = GetKernel(device, context, xclbin, kernelName);
	static size_t size_in_bytes = in.rows * in.cols * sizeof(unsigned char);

	cl_int err;
	OCL_CHECK(err,cl::Buffer buffer_in (context, CL_MEM_READ_ONLY, size_in_bytes, NULL , &err));
	OCL_CHECK(err,cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, size_in_bytes, NULL, &err));

	int arg =0;
    OCL_CHECK(err, err = krnl.setArg(arg++, buffer_in));
    OCL_CHECK(err, err = krnl.setArg(arg++, buffer_out));

    OCL_CHECK(err, err = krnl.setArg(arg++, thresh));
    OCL_CHECK(err, err = krnl.setArg(arg++, maxval));

    OCL_CHECK(err, err = krnl.setArg(arg++, in.rows));
    OCL_CHECK(err, err = krnl.setArg(arg++, in.cols));

	unsigned char *ptr_in = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_in, CL_TRUE, CL_MAP_WRITE, 0, size_in_bytes);
	memset(ptr_in, 0, size_in_bytes);
	for (unsigned int i = 0; i < size_in_bytes; i++) {
		ptr_in[i] = in.data[i];
	}

	out.data = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_out,CL_TRUE, CL_MAP_READ, 0, size_in_bytes);
	memset(out.data, 0, size_in_bytes);

	commandQueue.enqueueMigrateMemObjects( { buffer_in }, 0);
	commandQueue.enqueueTask(krnl);
	commandQueue.enqueueMigrateMemObjects( { buffer_out },CL_MIGRATE_MEM_OBJECT_HOST);
	commandQueue.finish();
}

