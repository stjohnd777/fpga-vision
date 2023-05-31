/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hls_stream.h"
#include "ap_int.h"

#include "../common/xf_common.hpp"
#include "../common/xf_utility.hpp"
#include "../imgproc/xf_custom_convolution.hpp"

#define GRAY 1
#define HEIGHT 1080
#define WIDTH  1920
#define FILTER_HEIGHT 3
#define FILTER_WIDTH 3
#define OUT_8U 1
#define XF_CV_DEPTH_IN_1  2
#define XF_CV_DEPTH_OUT_1 2

#define NPC1 XF_NPPC1
#define INTYPE XF_8UC1
#define OUTTYPE XF_8UC1
#define INPUT_PTR_WIDTH 8
#define OUTPUT_PTR_WIDTH 8

static constexpr int __XF_DEPTH_IN     = (HEIGHT * WIDTH* (XF_PIXELWIDTH(INTYPE,  NPC1)) / 8) / (INPUT_PTR_WIDTH / 8);
static constexpr int __XF_DEPTH_OUT    = (HEIGHT * WIDTH* (XF_PIXELWIDTH(OUTTYPE, NPC1)) / 8) / (OUTPUT_PTR_WIDTH / 8);
static constexpr int __XF_DEPTH_FILTER = FILTER_HEIGHT * FILTER_WIDTH;

extern "C" {

void accel_filter2dl(
		ap_uint<INPUT_PTR_WIDTH>* img_in,
		short int* filter,
		unsigned char shift,
		ap_uint<OUTPUT_PTR_WIDTH>* img_out,
		int rows,
		int cols) {

// clang-format off
#pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0 depth=__XF_DEPTH_IN
#pragma HLS INTERFACE m_axi      port=filter        offset=slave  bundle=gmem1 depth=__XF_DEPTH_FILTER
#pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem2 depth=__XF_DEPTH_OUT

#pragma HLS INTERFACE s_axilite  port=shift 			          bundle=control
#pragma HLS INTERFACE s_axilite  port=rows 			          	  bundle=control
#pragma HLS INTERFACE s_axilite  port=cols 			              bundle=control
#pragma HLS INTERFACE s_axilite  port=return 			          bundle=control
	// clang-format on

	xf::cv::Mat < INTYPE, HEIGHT, WIDTH, NPC1, XF_CV_DEPTH_IN_1> imgInput(rows, cols);
	xf::cv::Mat < OUTTYPE, HEIGHT, WIDTH, NPC1, XF_CV_DEPTH_OUT_1> imgOutput(rows, cols);

#pragma HLS DATAFLOW

	// Retrieve xf::cv::Mat objects from img_in data:
	xf::cv::Array2xfMat<INPUT_PTR_WIDTH, INTYPE, HEIGHT, WIDTH, NPC1,XF_CV_DEPTH_IN_1>(img_in, imgInput);

	// Run xfOpenCV kernel:
	xf::cv::filter2D<XF_BORDER_CONSTANT, FILTER_WIDTH, FILTER_HEIGHT, INTYPE,OUTTYPE, HEIGHT, WIDTH, NPC1, XF_CV_DEPTH_IN_1, XF_CV_DEPTH_OUT_1>
	(imgInput, imgOutput, filter, shift);

	// Convert _dst xf::cv::Mat object to output array:
	xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, OUTTYPE, HEIGHT, WIDTH, NPC1,XF_CV_DEPTH_OUT_1>
	(imgOutput, img_out);

	return;
} // End of kernel
}
