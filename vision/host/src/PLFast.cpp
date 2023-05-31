#include "PLFast.h"

#include "config/config.h"
#include "algo.h"
#include "myutils.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <string>

cv::Mat cvFast(cv::Mat& in_gray, float threshold, int rows, int cols) {
	std::vector < cv::KeyPoint > keypoints;
	cv::FAST(in_gray, keypoints, threshold, 1);
}

cv::Mat plFast(cv::Mat& in, unsigned char threshold, int rows, int cols) {

	static string xclbin = XCLBIN::FAST;
	static string kernelName = KERNEL_NAME::FAST;
	static cl::Device device = GetDevice();
	static cl::Context context(device);
	static cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);
	static cl::Kernel kernel = GetKernel(device, context, xclbin, kernelName);

//	if (in.rows != ROWS || in.cols != COLUMNS || in.depth != DEPTH) {
//		throw "Invalid Dimensions on Image ";
//	}

	size_t image_in_size_bytes = in.rows * in.cols * sizeof(unsigned char);
	size_t map_in_size_bytes = in.rows * in.cols * sizeof(float);
	size_t image_out_size_bytes = image_in_size_bytes;

	cv::Mat out(in.rows, in.cols, in.type());

	cl_int err;
	OCL_CHECK(err,cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
	OCL_CHECK(err,cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

	// Set kernel arguments:
	OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
	OCL_CHECK(err, err = kernel.setArg(1, threshold));
	OCL_CHECK(err, err = kernel.setArg(2, buffer_outImage));
	OCL_CHECK(err, err = kernel.setArg(3, rows));
	OCL_CHECK(err, err = kernel.setArg(4, cols));

	cl::Event event;
	OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inImage, // buffer on the FPGA
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
			image_out_size_bytes, out.data, // Data will be stored here
			nullptr, &event);

	// Clean up:
	queue.finish();

	return out;
}

//std::vector<cv::Point> extract(cv::Mat out_hls) {
//
//	std::vector < cv::Point > hls_points;
//	for (int j = 0; j < out_hls.rows; j++) {
//		for (int i = 0; i < out_hls.cols; i++) {
//			unsigned char value = out_hls.at<unsigned char>(j, i);
//			if (value != 0) {
//				short int y, x;
//				y = j;
//				x = i;
//
//				cv::Point tmp;
//				tmp.x = i;
//				tmp.y = j;
//
//				hls_points.push_back(tmp);
//				if (j > 0) {
//					cv::circle(out_img, cv::Point(x, y), 5,cv::Scalar(0, 0, 255, 255), 2, 8, 0);
//				}
//			}
//		}
//	}
//}
