#include "PLSobel.h"

const char* const XCLBIN_SOBEL8 = "binary_container_1.xclbin";
const char* const XCLBIN_SOBEL16 = "binary_container_1.xclbin";
const char* const KERNEL_NAME_SOBEL8 = "sobel_accel8";
const char* const KERNEL_NAME_SOBEL16 = "sobel_accel16";


#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <string>

void cvSobel(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width ){
    int scale = 1;
    int delta = 0;
    int FILTER_WIDTH = 7;
    int ddepth  = FILTER_WIDTH != 7 ? CV_8U : -1;
    cv::Sobel(in_img, grad_x, ddepth, 1, 0, FILTER_WIDTH, scale, delta, cv::BORDER_CONSTANT);
    cv::Sobel(in_img, grad_y, ddepth, 0, 1, FILTER_WIDTH, scale, delta, cv::BORDER_CONSTANT);
}

void plSobel8(cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width ){

	static string xclbin = XCLBIN_SOBEL8;
	static string kernelName = KERNEL_NAME_SOBEL8;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);


    cl::Buffer imageToDevice(context, 	 CL_MEM_READ_ONLY,  (height * width * 1));
    cl::Buffer imageFromDevice1(context, CL_MEM_WRITE_ONLY, (height * width * 1));
    cl::Buffer imageFromDevice2(context, CL_MEM_WRITE_ONLY, (height * width * 1));

    // Set the kernel arguments
    kernel.setArg(0, imageToDevice);
    kernel.setArg(1, imageFromDevice1);
    kernel.setArg(2, imageFromDevice2);
    kernel.setArg(3, height);
    kernel.setArg(4, width);

    queue.enqueueWriteBuffer(imageToDevice, CL_TRUE, 0, (height * width * DEPTH), in_img.data);

    // Profiling Objects
    cl_ulong start = 0;
    cl_ulong end = 0;
    double diff_prof = 0.0f;
    cl::Event event_sp;

    // Launch the kernel
    queue.enqueueTask(kernel, NULL, &event_sp);
    clWaitForEvents(1, (const cl_event*)&event_sp);

    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    diff_prof = end - start;
    std::cout << (diff_prof / 1000000) << "ms" << std::endl;

    queue.enqueueReadBuffer(imageFromDevice1, CL_TRUE, 0, (height * width * 1), grad_x.data);
    queue.enqueueReadBuffer(imageFromDevice2, CL_TRUE, 0, (height * width * 1), grad_y.data);
    queue.finish();

}

void plSobel16 (cv::Mat& in_img, cv::Mat& grad_x,cv::Mat& grad_y, int height, int width ){

	static string xclbin = XCLBIN_SOBEL16;
	static string kernelName = KERNEL_NAME_SOBEL16;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device,CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);


    cl::Buffer imageToDevice(context, 	 CL_MEM_READ_ONLY,  (height * width * 2));
    cl::Buffer imageFromDevice1(context, CL_MEM_WRITE_ONLY, (height * width * 2));
    cl::Buffer imageFromDevice2(context, CL_MEM_WRITE_ONLY, (height * width * 2));

    // Set the kernel arguments
    kernel.setArg(0, imageToDevice);
    kernel.setArg(1, imageFromDevice1);
    kernel.setArg(2, imageFromDevice2);
    kernel.setArg(3, height);
    kernel.setArg(4, width);

    queue.enqueueWriteBuffer(imageToDevice, CL_TRUE, 0, (height * width * DEPTH), in_img.data);

    // Profiling Objects
    cl_ulong start = 0;
    cl_ulong end = 0;
    double diff_prof = 0.0f;
    cl::Event event_sp;

    // Launch the kernel
    queue.enqueueTask(kernel, NULL, &event_sp);
    clWaitForEvents(1, (const cl_event*)&event_sp);

    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    diff_prof = end - start;
    std::cout << (diff_prof / 1000000) << "ms" << std::endl;

    queue.enqueueReadBuffer(imageFromDevice1, CL_TRUE, 0, (height * width * 2), grad_x.data);
    queue.enqueueReadBuffer(imageFromDevice2, CL_TRUE, 0, (height * width * 2), grad_y.data);
    queue.finish();

}

