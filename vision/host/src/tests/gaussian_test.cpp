#include <iostream>
#include <string>
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
using namespace cv;

#include "../PlGaussianFilter.h"

#include "../xcl2.hpp"

#define FILTER_WIDTH 3

// xf_params.hpp
#define TYPE 0
// XF_8UC1 = 0,
#define CH_TYPE 1
// XF_GRAY = 1

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

int gaussian_test(string path){

	cv::Mat in = cv::imread(path, 0);
    if (!in.data) {
        cout << "Failed to load the image ... !!!\n " << endl;
        return -1;
    }
    float sigma = 0.5f;
    auto out = plGaussian(in, sigma);
}

int gaussian_test_xilinx(int argc, char** argv) {

	if (argc != 2) {
        fprintf(stderr, "Usage: <executable> <input image path>\n");
        return -1;
    }

    cv::Mat in_img;
    cv::Mat out_img;
    cv::Mat ocv_ref;
    cv::Mat in_img_gau;
    cv::Mat in_gray;
    cv::Mat in_gray1;
	cv::Mat diff;

    in_img = cv::imread(argv[1], 0); // reading in the color image

    if (!in_img.data) {
        fprintf(stderr, "Failed to load the image ... !!!\n ");
        return -1;
    }

    out_img.create(in_img.rows, in_img.cols, CV_8UC1); // create memory for output image
    diff.create(in_img.rows, in_img.cols, CV_8UC1);    // create memory for OCV-ref image
    ocv_ref.create(in_img.rows, in_img.cols, CV_8UC1); // create memory for OCV-ref image


#if FILTER_WIDTH == 3
    float sigma = 0.5f;
#endif
#if FILTER_WIDTH == 7
    float sigma = 1.16666f;
#endif
#if FILTER_WIDTH == 5
    float sigma = 0.8333f;
#endif

    // OpenCV Gaussian filter function
    cv::GaussianBlur(in_img, ocv_ref, cv::Size(FILTER_WIDTH, FILTER_WIDTH), FILTER_WIDTH / 6.0, FILTER_WIDTH / 6.0, cv::BORDER_CONSTANT);

    imwrite("output_ocv.png", ocv_ref);

    /////////////////////////////////////// CL ////////////////////////

    int height = in_img.rows;
    int width = in_img.cols;
    std::cout << "Input image height : " << height << std::endl;
    std::cout << "Input image width  : " << width << std::endl;

    cl_int err;
    std::cout << "INFO: Running OpenCL section." << std::endl;

    // Get the device:
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Context, command queue and device name:
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
    OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

    //std::cout << "INFO: Device found - " << device_name << std::endl;

    // Load binary:

    std::string binaryFile = xcl::find_binary_file(device_name, "krnl_gaussian_filter");
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
    devices.resize(1);
    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));

    // Create a kernel:
    OCL_CHECK(err, cl::Kernel kernel(program, "gaussian_filter_accel", &err));

    // Allocate the buffers:
    OCL_CHECK(err, cl::Buffer imageToDevice(context, CL_MEM_READ_ONLY, (height * width * CH_TYPE), NULL, &err)); //,in_img.data);
    OCL_CHECK(err, cl::Buffer imageFromDevice(context, CL_MEM_WRITE_ONLY, (height * width * CH_TYPE), NULL,&err)); //,(ap_uint<OUTPUT_PTR_WIDTH>*)out_img.data);


    // Set kernel arguments:
    OCL_CHECK(err, err = kernel.setArg(0, imageToDevice));
    OCL_CHECK(err, err = kernel.setArg(1, imageFromDevice));
    OCL_CHECK(err, err = kernel.setArg(2, height));
    OCL_CHECK(err, err = kernel.setArg(3, width));
    OCL_CHECK(err, err = kernel.setArg(4, sigma));

    // Initialize the buffers:
    cl::Event event;

    OCL_CHECK(err, q.enqueueWriteBuffer(imageToDevice,              // buffer on the FPGA
                                        CL_TRUE,                    // blocking call
                                        0,                          // buffer offset in bytes
                                        (height * width * CH_TYPE), // Size in bytes
                                        in_img.data,                // Pointer to the data to copy
                                        nullptr, &event));

    // Profiling Objects
    cl_ulong start = 0;
    cl_ulong end = 0;
    double diff_prof = 0.0f;
    cl::Event event_sp;

    // Execute the kernel:
    OCL_CHECK(err, err = q.enqueueTask(kernel, NULL, &event_sp));

    clWaitForEvents(1, (const cl_event*)&event_sp);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    diff_prof = end - start;
    std::cout << (diff_prof / 1000000) << "ms" << std::endl;

    // Copy Result from Device Global Memory to Host Local Memory
    q.enqueueReadBuffer(imageFromDevice, // This buffers data will be read
                        CL_TRUE,         // blocking call
                        0,               // offset
                        (height * width * CH_TYPE),
                        out_img.data, // Data will be stored here
                        nullptr, &event_sp);

    q.finish();
    /////////////////////////////////////// end of CL ////////////////////////

    cv::imwrite("hw_out.jpg", out_img);

    //////////////////  Compute Absolute Difference ////////////////////
//    cv::absdiff(ocv_ref, out_img, diff);
//    cv::imwrite("out_error.jpg", diff);

//    float err_per;
//    xf::cv::analyzeDiff(diff, 0, err_per);
//
//    if (err_per > 1) {
//        fprintf(stderr, "\nTest Failed.\n ");
//        return -1;
//    } else {
//        std::cout << "Test Passed " << std::endl;
//        return 0;
//
}
