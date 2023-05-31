#define ROWS 1080
#define COLS 1920

extern "C" {

void krn_nuc(
		unsigned char* IMAGE,
		float* M,
		float* C,
		unsigned char rows,
		unsigned char cols,
		unsigned char* R) {

#pragma HLS INTERFACE m_axi      port=IMAGE   offset=slave  bundle=gmem0
#pragma HLS INTERFACE m_axi      port=M       offset=slave  bundle=gmem1
#pragma HLS INTERFACE m_axi      port=C       offset=slave  bundle=gmem2
#pragma HLS INTERFACE m_axi      port=R       offset=slave  bundle=gmem3
#pragma HLS INTERFACE s_axilite  port=rows
#pragma HLS INTERFACE s_axilite  port=cols
#pragma HLS INTERFACE s_axilite  port=return

	unsigned char r;
	unsigned char c;
	unsigned char k;

	for ( r = 0; r < rows; r++) {
#pragma HLS PIPELIME II=1
		for ( c = 0; c < cols; c++) {
#pragma HLS UNROLL factor=4
			R[r * cols + c] = IMAGE[r * cols + c] *  M[r * cols + c] +  C[r * cols + c];
		}
	}

}

}
