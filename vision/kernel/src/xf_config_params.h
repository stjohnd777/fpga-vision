
// Configure this based on the number of rows needed for the remap purpose
// e.g., If its a right to left flip two rows are enough

#define GRAY 1
#define RGB 0


///////////////////////////////////////////////////////////////////////////////////////////////
// remap TODO move to remap
//////////////////////////////////////////////////////////////////////////////////////////////
//#define XF_WIN_ROWS 8
// The type of interpolation, define "XF_REMAP_INTERPOLATION" as either "XF_INTERPOLATION_NN" or
// "XF_INTERPOLATION_BILINEAR"
//#define XF_REMAP_INTERPOLATION XF_INTERPOLATION_BILINEAR
//#define XF_USE_URAM false
//#define XF_CV_DEPTH_IN_1 2
//#define XF_CV_DEPTH_IN_2 2
//#define XF_CV_DEPTH_IN_3 2
//#define XF_CV_DEPTH_OUT 2

///////////////////////////////////////////////////////////////////////////////////////////////
// sobel
//////////////////////////////////////////////////////////////////////////////////////////////
#define MPC 0 // Multiple Pixels per Clock operation
#define SPC 1 // Single Pixel per Clock operation

/*  Set Filter size  */

#define FILTER_SIZE_3 1
#define FILTER_SIZE_5 0
#define FILTER_SIZE_7 0

//#define GRAY 1
//#define RGBA 0

#define INPUT_PTR_WIDTH 256
#define OUTPUT_PTR_WIDTH 256

#define XF_USE_URAM false

#define XF_CV_DEPTH_IN 2
#define XF_CV_DEPTH_OUT_0 2

#define MPC 0 // Multiple Pixels per Clock operation
#define SPC 1 // Single Pixel per Clock operation

/*  Set Filter size  */

#define FILTER_SIZE_3 1
#define FILTER_SIZE_5 0
#define FILTER_SIZE_7 0

#define GRAY 1
#define RGBA 0

//#define INPUT_PTR_WIDTH 256
//#define OUTPUT_PTR_WIDTH 256

#define XF_USE_URAM false

#define XF_CV_DEPTH_IN 2
#define XF_CV_DEPTH_OUT_0 2
#define XF_CV_DEPTH_OUT_1 2



