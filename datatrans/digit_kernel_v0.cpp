#include "digit.h"
#include "ap_int.h"

ap_uint<1> truncate(char x){
    return x.range(0);
}

struct tiff_frame{

}



extern "C"{
    void digit_kernel(char* input,
                      ap_uint<32>* output
                      //int total
                    )
    {
        #pragma HLS INTERFACE m_axi port=input bundle=gmem //max_read_burst_length=64

        #pragma HLS INTERFACE m_axi port=output bundle=gmem1

        #pragma HLS INTERFACE s_axilite port=input_v  bundle=gmem2 max_read_burst_length=32

        //+1 for line delimiter
        int npixel = entwidth * (entwidth + 1);

        

        int nchar = npixel + 3;
        ap_uint<nchar * 8> mem;
        ap_uint<1024> frame;
        ap_uint<32> tag;

        for (int loop_index = 0; loop_index < ndigit/2; loop_index++){
            #pragma HLS DATAFLOW 
            //read a struct, +3 for space, tag, and the line delimiter
            int read_offset = nchar * loop_index;
            for(int read_index = 0; read_index < nchar; read_index++){
                mem(nchar * 8 - 1 - read_index * 8, nchar * 8 - 1 - read_index * 8 - 8) = input[read_offset + read_index];
            }
        
            //process
            for(int i = 0; i < entheight; i++){
                #pragma HLS unroll
                for(int j = 0; j < entwidth; j++){
                    #pragma HLS unroll
                    frame(i * entwidth + j) = mem.range(0);

                }
                


            }

            //write
            for(int write_index = 0; write_index < npixel + 1; write_index++){

            }
                 
            
        }
       

    }
}