#include "./ncc.h"
#include <iostream>

using namespace std;

int main(){
    cout << "-------------------------------" << endl;
    cout << "Test started." << endl;
    data_t a[5][5] =   {{1,5,1,3,1},
                        {9,1,0,2,2},
                        {3,2,1,3,3},
                        {3,1,1,6,8},
                        {5,2,2,5,5}};
    data_t t[2][2] = {{1,1},{2,2}};
    result_t result[4][4];
    hls::stream<trans_pkt> input_img;
    hls::stream<trans_pkt> input_tmp;
    hls::stream<trans_pkt> output;
    trans_pkt data_p;
    for(int i=0; i<5; i++){
    	for(int j=0;j<5;j++){
    		data_p.data = a[i][j];
    		input_img.write(data_p);
    	}
    }
    data_p.data = 1;input_tmp.write(data_p);
    data_p.data = 1;input_tmp.write(data_p);
    data_p.data = 2;input_tmp.write(data_p);
    data_p.data = 2;input_tmp.write(data_p);
    ncc(input_img,input_tmp,output,2,2,5,5);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
        	data_p = output.read();
        	cout<<data_p.data<<endl;
        }
        cout<<endl;
    }

    cout << "Test passed." << endl;
    cout << "Test finished." << endl;
    cout << "-------------------------------" << endl;
    
}
