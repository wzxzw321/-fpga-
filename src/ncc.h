#include <hls_stream.h>
#include "ap_axi_sdata.h"
#include <stdint.h>
#include "ap_int.h"

#ifndef NCC_H_
#define NCC_H_

#define TMP_N 20
#define IMG_COLS 60

typedef ap_int<8> data_t;
typedef float result_t;
typedef ap_axiu<8, 0, 0, 0> trans_pkt;


void ncc(hls::stream<trans_pkt>& img, hls::stream<trans_pkt>& tmp,hls::stream<trans_pkt>& dst, int temlpate_rows, int template_cols, int img_rows, int img_cols);

#endif
