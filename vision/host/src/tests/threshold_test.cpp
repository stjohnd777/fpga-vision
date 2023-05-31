/*  set the type of thresholding*/


#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#include "../xcl2.hpp"


#include <string>
#include <iostream>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace std;

#include "../PLThreshold.h"

int thresholding_test (string path,unsigned char maxval = 255,unsigned char thresh = 128) {
	cv::Mat in = cv::imread(path,0);
    if (in.data == NULL) {
        cout << "Cannot open image " << path << endl;
        return -1;
    }
    cv::Mat out(in.rows, in.cols, in.depth());
	plThreshold(in,out);
	imwrite("hls_thresholding1_out.jpg", out);
}


int thresholding_test_xilinx (string path,unsigned char maxval = 50,unsigned char thresh = 100) {

    cv::Mat in_img;
    cv::Mat out_img;
    cv::Mat ocv_ref;
    cv::Mat in_gray;
    cv::Mat diff;

    //  reading in the color image
    in_img = cv::imread(path,0);
    if (in_img.data == NULL) {
        cout << "Cannot open image " << path << endl;
        return -1;
    }

    int height = in_img.rows;
    int width = in_img.cols;
    std::cout << "Input image height : " << height << std::endl;
    std::cout << "Input image width  : " << width << std::endl;

    ocv_ref.create(in_img.rows, in_img.cols, in_img.depth());
    out_img.create(in_img.rows, in_img.cols, in_img.depth());
    diff.create(in_img.rows, in_img.cols, in_img.depth());

    // OpenCV thresholding
    // #define THRESH_TYPE XF_THRESHOLD_TYPE_BINARY
    // XF_THRESHOLD_TYPE_BINARY = 0,
    cv::threshold(in_img, ocv_ref, thresh, maxval, 0);

    // OpenCL Section
    cl_int err;
    std::cout << "INFO: Running OpenCL section." << std::endl;

    // Device, Context, CommandQueue
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));

    std::string device_name = device.getInfo<CL_DEVICE_NAME>();
    std::string binaryFile = xcl::find_binary_file(device_name, "krnl_threshold");
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
    devices.resize(1);

    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));

    // Create a kernel:
    OCL_CHECK(err, cl::Kernel kernel(program, "threshold_accel", &err));

    // Allocate Bufferers
    OCL_CHECK(err, cl::Buffer imageToDevice(  context, CL_MEM_READ_ONLY,  (height * width), NULL, &err));
    OCL_CHECK(err, cl::Buffer imageFromDevice(context, CL_MEM_WRITE_ONLY, (height * width), NULL, &err));

    // Set the kernel arguments
    OCL_CHECK(err, err = kernel.setArg(0, imageToDevice));
    OCL_CHECK(err, err = kernel.setArg(1, imageFromDevice));
    OCL_CHECK(err, err = kernel.setArg(2, thresh));
    OCL_CHECK(err, err = kernel.setArg(3, maxval));
    OCL_CHECK(err, err = kernel.setArg(4, height));
    OCL_CHECK(err, err = kernel.setArg(5, width));


    OCL_CHECK(err, q.enqueueWriteBuffer(imageToDevice, CL_TRUE, 0, (height * width), in_img.data));

    // Profiling Objects
    cl_ulong start = 0;
    cl_ulong end = 0;
    double diff_prof = 0.0f;
    cl::Event event_sp;

    // Launch the kernel
    OCL_CHECK(err, err = q.enqueueTask(kernel, NULL, &event_sp));
    clWaitForEvents(1, (const cl_event*)&event_sp);
    // Get the profiling data
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    diff_prof = end - start;
    std::cout << (diff_prof / 1000000) << "ms" << std::endl;

    // Copying Device result data to Host memory
    q.enqueueReadBuffer(imageFromDevice, CL_TRUE, 0, (height * width), out_img.data);
    q.finish();

    // End OpenCL Write Results To File
    imwrite("hls_thresholding_out.jpg", out_img); // Write output image
    cv::absdiff(ocv_ref, out_img, diff);  // Compute absolute difference image
    imwrite("diff.png", diff);  // Save the difference image

    return 0;
}
