
extern "C" {

void image_thresholding_kernel00(unsigned char *input_image,
		unsigned char *output_image, unsigned int n, unsigned int m,
		unsigned int threshold, unsigned int maxVal) {

	unsigned char input_pixel_reg;
	unsigned char output_pixel_req;

	for (unsigned int i = 0; i < n * m; i++) {

		input_pixel_reg = input_image[i];
		output_pixel_req = (input_pixel_reg > threshold) ? maxVal : 0;
		output_image[i] = output_pixel_req;

	}

}

}
