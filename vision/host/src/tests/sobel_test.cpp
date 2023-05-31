
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "../xcl2.hpp"

//#define GRAY 1
//#define WIDTH 1920
//#define HEIGHT 1080
//#define FILTER_WIDTH 7
//#define PTYPE CV_8UC1
//#define CH_TYPE XF_GRAY
#define IN_TYPE 0


#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;

#include "../PLSobel.h"
int sobel_test(string path ) {

    cv::Mat in_img = cv::imread(path, 0);
    if (in_img.data == NULL) {
        cout << "Cannot open image at " << path << endl;
        return 0;
    }
    int height = in_img.rows;
    int width = in_img.cols;
    cv::Mat grad_x(in_img.rows, in_img.cols, CV_8UC1);
    cv::Mat grad_y(in_img.rows, in_img.cols, CV_8UC1);
    plSobel(in_img, grad_x, grad_y,  height,  width );
    imwrite("hls_gradx.jpg", grad_x);
    imwrite("hls_grady.jpg", grad_y);

    return 0;
}

int sobel_test_xilinx(string path ) {

    cv::Mat in_img;
    cv::Mat in_gray;
    cv::Mat diff;
    cv::Mat c_grad_x_1;
    cv::Mat c_grad_y_1;
    cv::Mat c_grad_x(in_img.rows, in_img.cols, CV_8UC1);
    cv::Mat c_grad_y(in_img.rows, in_img.cols, CV_8UC1);


    cv::Mat hls_grad_x(in_img.rows, in_img.cols, CV_8UC1);
    cv::Mat hls_grad_y(in_img.rows, in_img.cols, CV_8UC1);

    cv::Mat diff_grad_x(in_img.rows, in_img.cols, CV_8UC1);
    cv::Mat diff_grad_y(in_img.rows, in_img.cols, CV_8UC1);


    in_img = cv::imread(path, 0);
    if (in_img.data == NULL) {
        cout << "Cannot open image at " << path << endl;
        return 0;
    }
    int height = in_img.rows;
    int width = in_img.cols;
    std::cout << "Input image height : " << height << std::endl;
    std::cout << "Input image width  : " << width << std::endl;


    ///////////////// 	Opencv  Reference  ////////////////////////
    int scale = 1;
    int delta = 0;
    // TODO: #define mess from vitis sort out
    int FILTER_WIDTH = 7;
    int ddepth  = FILTER_WIDTH != 7 ? CV_8U : -1;
    cv::Sobel(in_img, c_grad_x_1, ddepth, 1, 0, FILTER_WIDTH, scale, delta, cv::BORDER_CONSTANT);
    cv::Sobel(in_img, c_grad_y_1, ddepth, 0, 1, FILTER_WIDTH, scale, delta, cv::BORDER_CONSTANT);
    imwrite("out_ocvx.jpg", c_grad_x_1);
    imwrite("out_ocvy.jpg", c_grad_y_1);


    /////////////////////////////////////// CL ////////////////////////
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];
    cl::Context context(device);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE);

    std::string device_name = device.getInfo<CL_DEVICE_NAME>();
    std::string binaryFile = xcl::find_binary_file(device_name, "krnl_sobel");
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
    devices.resize(1);
    cl::Program program(context, devices, bins);
    cl::Kernel krnl(program, "sobel_accel");

    cl::Buffer imageToDevice(context, 	 CL_MEM_READ_ONLY,  (height * width * 1));
    cl::Buffer imageFromDevice1(context, CL_MEM_WRITE_ONLY, (height * width * 1));
    cl::Buffer imageFromDevice2(context, CL_MEM_WRITE_ONLY, (height * width * 1));

    // Set the kernel arguments
    krnl.setArg(0, imageToDevice);
    krnl.setArg(1, imageFromDevice1);
    krnl.setArg(2, imageFromDevice2);
    krnl.setArg(3, height);
    krnl.setArg(4, width);

    q.enqueueWriteBuffer(imageToDevice, CL_TRUE, 0, (height * width * 1), in_img.data);

    // Profiling Objects
    cl_ulong start = 0;
    cl_ulong end = 0;
    double diff_prof = 0.0f;
    cl::Event event_sp;

    printf("before kernel .... !!!\n");
    // Launch the kernel
    q.enqueueTask(krnl, NULL, &event_sp);
    clWaitForEvents(1, (const cl_event*)&event_sp);
    printf("after kernel .... !!!\n");

    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    event_sp.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    diff_prof = end - start;
    std::cout << (diff_prof / 1000000) << "ms" << std::endl;

    q.enqueueReadBuffer(imageFromDevice1, CL_TRUE, 0, (height * width * 1), hls_grad_x.data);
    q.enqueueReadBuffer(imageFromDevice2, CL_TRUE, 0, (height * width * 1), hls_grad_y.data);
    q.finish();

    imwrite("out_errorx.jpg", hls_grad_x);
    imwrite("out_errory.jpg", hls_grad_y);

    return 0;
}




