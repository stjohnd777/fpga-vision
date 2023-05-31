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

#ifndef _XF_THRESHOLD_CONFIG_H_
#define _XF_THRESHOLD_CONFIG_H_

#include "hls_stream.h"
#include "ap_int.h"

#include "../common/xf_common.hpp"
#include "../common/xf_utility.hpp"
#include "../imgproc/xf_threshold.hpp"

//#include "../xf_config_params.h"

typedef ap_uint<8> ap_uint8_t;
typedef ap_uint<64> ap_uint64_t;


/*  set the type of thresholding*/
#define SPC 1 // Single Pixel per Clock operation
#define MPC 0 // Multiple Pixels per Clock operation


#define THRESH_TYPE XF_THRESHOLD_TYPE_BINARY
#define HEIGHT 1080
#define WIDTH 1920
#define INPUT_PTR_WIDTH 256
#define OUTPUT_PTR_WIDTH 256
#define XF_CV_DEPTH_IN  1
#define XF_CV_DEPTH_OUT 1
#define NPIX XF_NPPC1


void threshold_accel(xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _src,
                     xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _dst,
                     unsigned char thresh,
                     unsigned char maxval);

#endif // end of _XF_THRESHOLD_CONFIG_H_
