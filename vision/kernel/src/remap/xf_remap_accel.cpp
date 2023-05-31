
#include "xf_remap_config.h"

// Configure this based on the number of rows needed for the remap purpose
// e.g., If its a right to left flip two rows are enough
#define XF_WIN_ROWS 8
// The type of interpolation, define "XF_REMAP_INTERPOLATION" as either "XF_INTERPOLATION_NN" or
// "XF_INTERPOLATION_BILINEAR"
#define XF_INTERPOLATION_TYPE 	XF_INTERPOLATION_BILINEAR
#define XF_CV_DEPTH_IN_1 2
#define XF_CV_DEPTH_IN_2 2
#define XF_CV_DEPTH_IN_3 2
#define XF_CV_DEPTH_OUT  2
#define XF_USE_URAM false

#define PTR_IMG_WIDTH 			8

#define TYPE_XY 				XF_32FC1
#define PTR_MAP_WIDTH 			32

#define CHANNELS 				1

#define TYPE 					XF_8UC1
#define HEIGHT 					1080
#define WIDTH 					1920
#define NPC 					XF_NPPC1


extern "C" {

void remap_accel(
    ap_uint<PTR_IMG_WIDTH>* img_in, // uchar* or ap_uint<12>* or ushort*
	float* map_x,
	float* map_y,
	ap_uint<PTR_IMG_WIDTH>* img_out,  // uchar*
	int rows,
	int cols)
{
// clang-format off
    #pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0
    #pragma HLS INTERFACE m_axi      port=map_x         offset=slave  bundle=gmem1
    #pragma HLS INTERFACE m_axi      port=map_y         offset=slave  bundle=gmem2
    #pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem3
    #pragma HLS INTERFACE s_axilite  port=rows 	
    #pragma HLS INTERFACE s_axilite  port=cols 	
    #pragma HLS INTERFACE s_axilite  port=return
    // clang-format on

    xf::cv::Mat<TYPE, 	HEIGHT, WIDTH, NPC, XF_CV_DEPTH_IN_1> 	imgInput(rows, cols);
    xf::cv::Mat<TYPE_XY,HEIGHT, WIDTH, NPC, XF_CV_DEPTH_IN_2> 	mapX(rows, cols);
    xf::cv::Mat<TYPE_XY,HEIGHT, WIDTH, NPC, XF_CV_DEPTH_IN_3> 	mapY(rows, cols);
    xf::cv::Mat<TYPE, 	HEIGHT, WIDTH, NPC, XF_CV_DEPTH_OUT> 		imgOutput(rows, cols);

    const int HEIGHT_WIDTH_LOOPCOUNT = HEIGHT * WIDTH / XF_NPIXPERCYCLE(NPC);
	for (unsigned int i = 0; i < rows * cols; ++i) {
// clang-format off
	#pragma HLS LOOP_TRIPCOUNT min=1 max=HEIGHT_WIDTH_LOOPCOUNT
        #pragma HLS PIPELINE II=1
        // clang-format on
        float map_x_val = map_x[i];
        float map_y_val = map_y[i];
        mapX.write_float(i, map_x_val);
        mapY.write_float(i, map_y_val);
    }

// clang-format off
// clang-format on

// clang-format off
    #pragma HLS DATAFLOW
    // clang-format on

    // Retrieve xf::cv::Mat objects from img_in data:
    xf::cv::Array2xfMat<PTR_IMG_WIDTH, TYPE, HEIGHT, WIDTH, NPC, XF_CV_DEPTH_IN_1>(img_in, imgInput);

    // Run xfOpenCV kernel:
    xf::cv::remap<XF_WIN_ROWS, XF_INTERPOLATION_TYPE, TYPE, TYPE_XY, TYPE, HEIGHT, WIDTH, NPC, XF_USE_URAM,  XF_CV_DEPTH_IN_1, XF_CV_DEPTH_IN_2, XF_CV_DEPTH_IN_3, XF_CV_DEPTH_OUT>
    (imgInput, imgOutput, mapX, mapY);

    // Convert _dst xf::cv::Mat object to output array:
    xf::cv::xfMat2Array<PTR_IMG_WIDTH, TYPE, HEIGHT, WIDTH, NPC, XF_CV_DEPTH_OUT>(imgOutput, img_out);

    return;
} // End of kernel

} // End of extern C
