#pragma once

enum Algo {
	Canny = 1, Sobel, Thresholding, MyThresholding, Remap, Gaussian
};

namespace XCLBIN {
const char* const MY_THRESH_0 = "binary_container_1.xclbin";
const char* const MY_THRESH_1 = "binary_container_1.xclbin";
const char* const MY_THRESH_2 = "binary_container_1.xclbin";
const char* const REMAP = "binary_container_1.xclbin";
const char* const GAUSSIAN = "binary_container_1.xclbin";
const char* const FAST = "binary_container_1.xclbin";
const char* const SOBEL = "binary_container_1.xclbin";
const char* const MEDIAN = "binary_container_1.xclbin";
const char* const BILATERAL = "binary_container_1.xclbin";
const char* const NUC = "binary_container_1.xclbin";
} ;
namespace KERNEL_NAME {
const char* const MY_THRESHOLD_0 = "image_thresholding_kernel00";
const char* const MY_THRESHOLD_1 = "image_thresholding_kernel01";
const char* const MY_THRESHOLD_2 = "image_thresholding_kernel02";
const char* const REMAP = "remap_accel";
const char* const GAUSSIAN = "gaussian_filter_accel";
const char* const FAST = "fast_accel";
const char* const SOBEL = "sobel_accel";
const char* const MEDIAN = "median_blur_accel";
const char* const BILATERAL = "bilateral_filter_accel";
const char* const NUC = "krn_nuc";
} ;
