#include "digit.h"
#include "ap_int.h"



extern "C"{
    void digit_kernel(ap_uint<8>    *input,
                      ap_uint<32> *output
                      //int total
                    )
    {
        #pragma HLS INTERFACE m_axi port=input bundle=gmem //max_read_burst_length=64

        #pragma HLS INTERFACE m_axi port=output bundle=gmem1

        #pragma HLS INTERFACE s_axilite port=input_v  bundle=gmem2 max_read_burst_length=32

        //+1 for line delimiter
        // int npixel = entheight * (entwidth + 1);
        // int nchar = npixel + 3;
        // int nbit = nchar * 8;

        //char inframe[entheight][entwidth+1];
        ap_uint<8> inframe[npixel];
        #pragma HLS ARRAY_PARTITION variable=inframe complete

        ap_uint<8> intag[3];
        #pragma HLS ARRAY_PARTITION variable=intag complete

        ap_uint<1024> frame = 0;
        ap_uint<32> tag = 0;
        

        for (int loop_index = 0; loop_index < ndigit; loop_index++){
            #pragma HLS DATAFLOW 
            //read a struct, +3 for space, tag, and the line delimiter
            int read_offset = nchar * loop_index;
            for(int read_index = 0; read_index < nchar; read_index++){
                ap_uint<8> in = input[read_offset + read_index];
                if (read_index < npixel){
                    inframe[read_index] = in;
                }
                else{
                    intag[read_index - npixel] = in;
                }
                
            }
        
            //access & process
            for(int i = 0; i < entheight; i++){
                #pragma HLS unroll
                for(int j = 0; j < entwidth; j++){
                    #pragma HLS unroll
                    ap_uint<8> pixel = inframe[i * (entwidth + 1) + j];
                    frame.range(1023 - (i * entwidth + j), 1023 - (i * entwidth + j)) = pixel.range(0,0);
                }
            }
            tag.range(7, 0) = intag[1];

            //write
            int write_offset = loop_index * (entheight + 1);
            for(int i = 0; i < entheight + 1; i++){
                ap_int<32> out;
                if (i < entheight){
                    out = frame.range(1023 - i * entwidth, 1024 - (i + 1) * entwidth);
                }
                else{
                    out = tag;
                }
                output[write_offset + i] = out;
            }
                   
        }
       

    }
}