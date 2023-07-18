#include "./ncc.h"
#include <math.h>


//flag 0:tmp 1:win
result_t avrage(data_t src[TMP_N][TMP_N], int rows, int cols){
#pragma HLS ARRAY_PARTITION variable=src dim=2 complete
    result_t sum=0;
    result_t avg;
    data_t sum_local[TMP_N];
#pragma HLS ARRAY_PARTITION variable=sum_local complete
    for (int i = 0; i < cols; i++)
        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS PIPELINE II=1
            sum_local[i] = 0;
        }

    for (int i = 0; i < cols; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
        for (int j = 0; j < rows; j++)
        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS PIPELINE II=1
        	sum_local[j] += src[i][j];
        }
    }
	for(int jj=0; jj<cols; jj++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
		sum += sum_local[jj];
	}
    avg = sum/rows/cols;
    return avg;
}

result_t var(data_t src[TMP_N][TMP_N], int rows, int cols, result_t avg){
#pragma HLS ARRAY_PARTITION variable=src dim=2 complete
    result_t sum = 0;
    result_t var = 0;

	for (int i = 0; i < rows; i++)
	{
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
		for (int j = 0; j < cols; j++)
		{
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS PIPELINE II=1
			sum += (src[i][j] - avg)*(src[i][j] - avg);
		}
	}

    var = sum/rows/cols;
    return sqrt(var);
}

void init_tmp(data_t tmp_board[TMP_N][TMP_N], hls::stream<trans_pkt>& tmp, int temlpate_rows, int template_cols){
#pragma HLS ARRAY_PARTITION variable=tmp_board complete dim=2
	trans_pkt data_p;
	for (int i = 0; i < temlpate_rows; i++)
	    {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
	        for (int j = 0; j < template_cols; j++)
	        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS pipeline II=1
	        	data_p = tmp.read();
	            tmp_board[i][j] = data_p.data;
	        }
	    }
}

void init_line_buffer(data_t line_buffer[TMP_N][IMG_COLS], hls::stream<trans_pkt>& img, int temlpate_rows, int img_cols){
#pragma HLS ARRAY_PARTITION variable=line_buffer complete dim=2
	trans_pkt data_p;
	for (int i = 1; i < temlpate_rows; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
        for (int j = 0; j < img_cols; j++)
        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=60
#pragma HLS pipeline II=1
        	data_p = img.read();
            line_buffer[i][j] = data_p.data;
        }
    }
}

result_t ncc_cal(data_t win[TMP_N][TMP_N], data_t tmp[TMP_N][TMP_N], int rows, int cols, result_t tmp_avg, result_t tmp_var){
#pragma HLS ARRAY_PARTITION variable=win dim=0 complete
#pragma HLS ARRAY_PARTITION variable=tmp dim=0 complete
	//window平均值�?�方�?
    result_t win_avg = avrage(win, rows, cols);
    result_t win_var = var(win, rows, cols, win_avg);
    result_t sum =0;
    result_t under = tmp_var *win_var;

    for (int i = 0; i < rows; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS PIPELINE
        for (int j = 0; j < cols; j++)
        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS UNROLL
        	sum += (tmp[i][j]  - tmp_avg)*(win[i][j] - win_avg)/ under;
        }
    }
    return sum/rows/cols;
}

void ncc(hls::stream<trans_pkt>& img, hls::stream<trans_pkt>& tmp,hls::stream<trans_pkt>& dst, int temlpate_rows, int template_cols, int img_rows, int img_cols){
#pragma HLS INTERFACE axis port=img
#pragma HLS INTERFACE axis port=tmp
#pragma HLS INTERFACE axis port=dst
#pragma HLS INTERFACE s_axilite port=temlpate_rows  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=template_cols  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=img_rows  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=img_cols  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    trans_pkt data_p;
    result_t ncc_res;
    data_t tmp_board[TMP_N][TMP_N];
    data_t win_board[TMP_N][TMP_N];

    data_t line_buffer[TMP_N][IMG_COLS];

//#pragma HLS ARRAY_PARTITION variable=tmp_board complete dim=2
#pragma HLS ARRAY_PARTITION variable=win_board complete dim=2
#pragma HLS ARRAY_PARTITION variable=line_buffer dim=2 complete

    //读入tmp,读取line_buffer
	init_tmp(tmp_board, tmp, temlpate_rows, template_cols);
	init_line_buffer(line_buffer, img, temlpate_rows, img_cols);

    //模版平均值�?�方�?
    result_t tmp_avg = avrage(tmp_board, temlpate_rows, template_cols);
    result_t tmp_var = var(tmp_board, temlpate_rows, template_cols, tmp_avg);

    //原图像的每个像素
	for (int i = 0; i < img_rows - temlpate_rows +1; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=41

		for (int line_i = 0; line_i < img_cols; line_i++)
		    {
#pragma HLS LOOP_TRIPCOUNT min=1 max=60
		        for (int line_j = 0; line_j < temlpate_rows-1; line_j++)
		        {
#pragma HLS LOOP_TRIPCOUNT min=1 max=19
#pragma HLS PIPELINE II=1
		        	line_buffer[line_j][line_i] = line_buffer[line_j+1][line_i];
		        }
		    }
		for (int line_j = 0; line_j < img_cols; line_j++)
			{
#pragma HLS LOOP_TRIPCOUNT min=1 max=60
#pragma HLS pipeline II=1
	        	data_p = img.read();
	        	line_buffer[temlpate_rows-1][line_j] = data_p.data;
			}

		for (int j = 0; j < img_cols - template_cols +1; j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=39
            //每一个像�?
            //读取window
            for (int k = 0; k < temlpate_rows; k++)
            {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS PIPELINE II=1
                for (int l = 0; l < template_cols; l++)
                {
#pragma HLS LOOP_TRIPCOUNT min=1 max=20
#pragma HLS UNROLL
                	win_board[k][l] = line_buffer[k][j+l];
                }
            }

			//�?始匹�?
            result_t out = ncc_cal(win_board, tmp_board, temlpate_rows, template_cols, tmp_avg, tmp_var);
			data_p.data = (out+1)/2*255;
			dst.write(data_p);
		}
	}
}
