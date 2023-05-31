#include "common/xf_params.hpp"
#include "imgproc/xf_remap.hpp"

//	Number of input image rows to be buffered inside.
// Must be set based on the map data. For instance, for left right flip, 2 rows are sufficient.
#define XF_WIN_ROWS 				8
#define HEIGHT_L_16 				1944
#define WIDTH_L_16 					2592
// 	Data width of the input pointer. The value must be power 2, starting from 8 to 512.
#define PTR_IMG_WIDTH_R16 			16
//Number of pixels computed in parallel. Example XF_NPPC1, XF_NPPC8
#define NPC_L16 					XF_NPPC1
// Type of interpolation, either XF_INTERPOLATION_NN (nearest neighbor) or XF_INTERPOLATION_BILINEAR (linear interpolation)
#define XF_INTERPOLATION_TYPE_L16 	XF_INTERPOLATION_BILINEAR
// 	Enable to map some structures to UltraRAM instead of BRAM.
#define XF_USE_URAM_L16 false
//XFCVDEPTH	Depth of the Output image.
//XFCVDEPTH_IN	Depth of the input image.
//XFCVDEPTH_Remapped	Depth of the output image.
//XFCVDEPTH_MAPX	Depth of the input image.
//XFCVDEPTH_MAPY	Depth of the output image.
#define XF_CV_DEPTH_IN_1_L16 2
#define XF_CV_DEPTH_IN_2_L16 2
#define XF_CV_DEPTH_IN_3_L16 2
#define XF_CV_DEPTH_OUT_L16  2
// Input Mat type. Example XF_8UC1, XF_16UC1, XF_8UC3 and XF_8UC4
#define TYPE_L16 					XF_16UC1
#define TYPE_XY_L16 				XF_32FC1
// 	Data width of the input pointer MAPS
#define PTR_MAP_WIDTH_L16 			32


//The remap function takes pixels from one place in the image and relocates
// them to another position in another image.
// Two types of interpolation methods are used here for mapping the image from source to destination image.
extern "C" {

void remap_accel_L16(ap_uint<PTR_IMG_WIDTH_R16>* img_in, float* map_x, float* map_y, ap_uint<PTR_IMG_WIDTH_R16>* img_out, int rows, int cols) {
// clang-format off
#pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0
#pragma HLS INTERFACE m_axi      port=map_x         offset=slave  bundle=gmem1
#pragma HLS INTERFACE m_axi      port=map_y         offset=slave  bundle=gmem2
#pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem3
#pragma HLS INTERFACE s_axilite  port=rows
#pragma HLS INTERFACE s_axilite  port=cols
#pragma HLS INTERFACE s_axilite  port=return
	// clang-format on

	xf::cv::Mat < TYPE_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_CV_DEPTH_IN_1_L16 > imgInput(rows, cols);
	xf::cv::Mat < TYPE_XY_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_CV_DEPTH_IN_2_L16 > mapX(rows, cols);
	xf::cv::Mat < TYPE_XY_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_CV_DEPTH_IN_3_L16 > mapY(rows, cols);
	xf::cv::Mat < TYPE_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_CV_DEPTH_OUT_L16 > imgOutput(rows, cols);

	const int HEIGHT_WIDTH_LOOPCOUNT = HEIGHT_L_16 * WIDTH_L_16 / XF_NPIXPERCYCLE(NPC_L16);

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
	xf::cv::Array2xfMat<PTR_IMG_WIDTH_R16, TYPE_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16,XF_CV_DEPTH_IN_1_L16>(img_in, imgInput);

	// Run xfOpenCV kernel:
	xf::cv::remap<XF_WIN_ROWS, XF_INTERPOLATION_TYPE_L16, TYPE_L16, TYPE_XY_L16, TYPE_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_USE_URAM_L16, XF_CV_DEPTH_IN_1_L16, XF_CV_DEPTH_IN_2_L16, XF_CV_DEPTH_IN_3_L16, XF_CV_DEPTH_OUT_L16>( imgInput, imgOutput, mapX, mapY);

	// Convert _dst xf::cv::Mat object to output array:
	xf::cv::xfMat2Array<PTR_IMG_WIDTH_R16, TYPE_L16, HEIGHT_L_16, WIDTH_L_16, NPC_L16, XF_CV_DEPTH_OUT_L16>( imgOutput, img_out);

	return;
} // End of kernel

} // End of extern C
