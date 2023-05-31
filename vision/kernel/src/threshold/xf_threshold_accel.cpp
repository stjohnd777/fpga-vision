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

#include "xf_threshold_config.h"

//#define THRESH_TYPE XF_THRESHOLD_TYPE_BINARY
//#define HEIGHT 1080
//#define WIDTH 1920
//#define NPC1 XF_NPPC1
//#define NPIX XF_NPPC1
//#define INPUT_PTR_WIDTH 256
//#define OUTPUT_PTR_WIDTH 256
//#define XF_CV_DEPTH_IN  1
//#define XF_CV_DEPTH_OUT 1

#include "hls_stream.h"
#include "ap_int.h"

#include "../common/xf_common.hpp"
#include "../common/xf_utility.hpp"
#include "../imgproc/xf_threshold.hpp"


extern "C" {
void threshold_accel(ap_uint<INPUT_PTR_WIDTH>* img_inp,
                     ap_uint<OUTPUT_PTR_WIDTH>* img_out,
                     unsigned char thresh,
                     unsigned char maxval,
                     int rows,
                     int cols) {
// clang-format off
    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi     port=img_out  offset=slave bundle=gmem2

    #pragma HLS INTERFACE s_axilite port=thresh     
    #pragma HLS INTERFACE s_axilite port=maxval     
    #pragma HLS INTERFACE s_axilite port=rows     
    #pragma HLS INTERFACE s_axilite port=cols     
    #pragma HLS INTERFACE s_axilite port=return
    // clang-format on

    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPIX;

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX, XF_CV_DEPTH_IN> in_mat(rows, cols);
    // clang-format off
    // clang-format on

    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX, XF_CV_DEPTH_OUT> out_mat(rows, cols);
// clang-format off
// clang-format on

// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX, XF_CV_DEPTH_IN>(img_inp, in_mat);

    xf::cv::Threshold<THRESH_TYPE, XF_8UC1, HEIGHT, WIDTH, NPIX, XF_CV_DEPTH_IN, XF_CV_DEPTH_OUT>(in_mat, out_mat,thresh, maxval);

    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX, XF_CV_DEPTH_OUT>(out_mat, img_out);
}
}