#include "PLMyThresholding.h"

#include "config/config.h"
#include "algo.h"
#include"myutils.h"
#include <string>
#include <opencv2/core/core.hpp>
#include <stdlib.h>

using namespace std;
using namespace cv;


void plMyThresh00(Mat& in, Mat& out, unsigned int threshold_value ,unsigned int max_binary_value) {

	static string xclbin = XCLBIN::MY_THRESH_0;
	static string kernelName = KERNEL_NAME::MY_THRESHOLD_0;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue commandQueue(context, device,
	CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel krnl = GetKernel(device, context, xclbin, kernelName);
	static size_t size_in_bytes = ROWS * COLUMNS * sizeof(unsigned char);

	cl_int err;
	OCL_CHECK(err,cl::Buffer buffer_in (context, CL_MEM_READ_ONLY, size_in_bytes, NULL , &err));
	OCL_CHECK(err,cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, size_in_bytes, NULL, &err));

	int narg = 0;
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_in));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_out));
	OCL_CHECK(err, err = krnl.setArg(narg++, in.cols));
	OCL_CHECK(err, err = krnl.setArg(narg++, in.rows));
	OCL_CHECK(err, err = krnl.setArg(narg++, threshold_value));
	OCL_CHECK(err, err = krnl.setArg(narg++, max_binary_value));

	unsigned char *ptr_in = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_in, CL_TRUE, CL_MAP_WRITE, 0, size_in_bytes);
	memset(ptr_in, 0, size_in_bytes);
	for (unsigned int i = 0; i < size_in_bytes; i++) {
		ptr_in[i] = in.data[i];
	}

	out.data = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_out,
	CL_TRUE, CL_MAP_READ, 0, size_in_bytes);
	memset(out.data, 0, size_in_bytes);

	commandQueue.enqueueMigrateMemObjects( { buffer_in }, 0);
	commandQueue.enqueueTask(krnl);
	commandQueue.enqueueMigrateMemObjects( { buffer_out },
	CL_MIGRATE_MEM_OBJECT_HOST);
	commandQueue.finish();
}

void plMyThresh01(Mat& in, Mat& out, unsigned int threshold_value ,unsigned int max_binary_value) {

	static string xclbin = "binary_container_1.xclbin";
	static string kernelName = "image_thresholding_kernel01";
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue commandQueue(context, device,
	CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel krnl = GetKernel(device, context, xclbin, kernelName);
	static size_t size_in_bytes = ROWS * COLUMNS * sizeof(unsigned char);

	cl_int err;
	OCL_CHECK(err,
			cl::Buffer buffer_in (context, CL_MEM_READ_ONLY, size_in_bytes, NULL , &err));
	OCL_CHECK(err,
			cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, size_in_bytes, NULL, &err));

	int narg = 0;
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_in));
	OCL_CHECK(err, err = krnl.setArg(narg++, buffer_out));
	OCL_CHECK(err, err = krnl.setArg(narg++, in.cols));
	OCL_CHECK(err, err = krnl.setArg(narg++, in.rows));
	OCL_CHECK(err, err = krnl.setArg(narg++, threshold_value));
	OCL_CHECK(err, err = krnl.setArg(narg++, max_binary_value));

	unsigned char *ptr_in = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_in, CL_TRUE, CL_MAP_WRITE, 0, size_in_bytes);
	memset(ptr_in, 0, size_in_bytes);
	for (unsigned int i = 0; i < size_in_bytes; i++) {
		ptr_in[i] = in.data[i];
	}

	out.data = (unsigned char *) commandQueue.enqueueMapBuffer(buffer_out,
	CL_TRUE, CL_MAP_READ, 0, size_in_bytes);
	memset(out.data, 0, size_in_bytes);

	commandQueue.enqueueMigrateMemObjects( { buffer_in }, 0);
	commandQueue.enqueueTask(krnl);
	commandQueue.enqueueMigrateMemObjects( { buffer_out },
	CL_MIGRATE_MEM_OBJECT_HOST);
	commandQueue.finish();
}



