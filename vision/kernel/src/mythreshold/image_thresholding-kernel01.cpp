

extern "C" {

void image_thresholding_kernel01(unsigned char *input_image,
		unsigned char *output_image, unsigned int n, unsigned int m,
		unsigned int threshold, unsigned int maxVal) {

#define _ROWS 1080
#define _COLS 1920
// --connectivity.sp image_thresholding_kernel01_1.m_axi_gmem0:HP0  --connectivity.sp image_thresholding_kernel01_1.m_axi_gmem1:HP1
#pragma HLS INTERFACE m_axi  port=input_image   offset=slave  bundle=gmem0
#pragma HLS INTERFACE m_axi  port=output_image  offset=slave  bundle=gmem1

	unsigned char input_pixel_reg;
	unsigned char output_pixel_req;

	unsigned char inBRAM[_ROWS * _COLS];
	unsigned char outBRAM[_ROWS * _COLS];

	for (int i = 0; i < _COLS * _ROWS; i++) {
#pragma HLS unroll factor=10
		inBRAM[i] = input_image[i];
	}

	for (unsigned int i = 0; i < 1920 * 1080; i++) {
#pragma HLS unroll factor=10
		input_pixel_reg = input_image[i];
		output_pixel_req = input_pixel_reg > threshold ? maxVal : 0;
		outBRAM[i] = output_pixel_req;
	}

	for (int i = 0; i < _COLS * _ROWS; i++) {
#pragma HLS unroll factor=10
		output_image[i] = outBRAM[i];
	}

}

}
