#include "digit.h"
#include <ap_int.h>
#include <hls_stream.h>


static void read_input(ap_uint<8> *input, hls::stream<ap_uint<8>> &inStream, int offset) {
mem_rd:
    for (int i = 0; i < nchar; i++) {
       #pragma HLS PIPELINE II=1
       //#pragma HLS LOOP_TRIPCOUNT min=nchar max=nchar
        //Blocking write command to inStream
        inStream << input[offset + i];
    }
}

static void truncate(hls::stream<ap_uint<8>> &inStream,
                  hls::stream<ap_uint<1>> &outStream
                ) {
// execute:
//     for (int i = 0; i < size; i++) {
//        #pragma HLS PIPELINE II=1
//        //#pragma HLS LOOP_TRIPCOUNT min=ndigit max=ndigit
//         int inVar = inStream.read();
//         int adderedVal = inVar + incr;
//         //Blocking read command from inStream and Blocking write command
//         //to outStream
//         outStream << adderedVal;
//     }
//ap_uint<33> tag = 0;
ap_uint<1> intag = 0;
process1:
    for(int i = 0; i < nchar; i++){
        #pragma HLS PIPELINE II=1
        ap_uint<8> inVar = inStream.read();
        ap_uint<1> out = inVar.range(0,0);
        // if (i == nchar - 2)
        //     intag = inVar;
        if (i < npixel)
            outStream << out;
        else
            outStream  << intag;
    }
    // //tag.range(32, 26) = intag;
    // for(int i = 7; i >= 0; i--){
    //     #pragma HLS PIPELINE II=1
    //     outStream << intag.range(i, i);
    // }
}



static void write_result(ap_uint<1> *output, hls::stream<ap_uint<1>> &outStream, int offset) {
mem_wr:
    for (int i = 0; i < nchar; i++) {
       #pragma HLS PIPELINE II=1
       //#pragma HLS LOOP_TRIPCOUNT min=c_size max=c_size
        //Blocking read command from OutStream
        output[offset + i] = outStream.read();
    }
}

extern "C"{
    void digit_kernel(ap_uint<8>    *input,
                      ap_uint<1> *output
                      //int total
                    )
    {
        #pragma HLS INTERFACE m_axi port=input bundle=gmem max_read_burst_length=256

        #pragma HLS INTERFACE m_axi port=output bundle=gmem1 max_write_burst_length=256


        //+1 for line delimiter
        // int npixel = entheight * (entwidth + 1);
        // int nchar = npixel + 3;
        // int nbit = nchar * 8;


        //may change to 32 by 32 2D array to reduce resource usage, + 1 for tag and line breaker
        // ap_uint<8> inframe[entheight][entwidth + 1];
        // #pragma HLS ARRAY_PARTITION variable=inframe complete

        // ap_uint<8> inframe[npixel];
        // #pragma HLS array_reshape variable=inframe type=block factor=33 dim=0
        // //#pragma HLS ARRAY_PARTITION variable=inframe complete
        // ap_uint<8> intag[3];
        // #pragma HLS ARRAY_PARTITION variable=intag complete
        

        // ap_uint<32> frame[32];
        // ap_uint<32> tag = 0;
        static hls::stream<ap_uint<8>> instream;
        static hls::stream<ap_uint<1>> outstream;

        //static hls::stream<int> streamArray[STAGES + 1];
        for (int loop_index = 0; loop_index < ndigit; loop_index++){
            #pragma HLS DATAFLOW 
            #pragma HLS dependence intag type=inter dependent=false
            #pragma HLS dependence inframe type=inter dependent=false
            //read 
            int input_offset = nchar * loop_index;
            int output_offset = nchar * loop_index;
            read_input(input, instream, input_offset);
            truncate(instream, outstream);
            write_result(output, outstream, output_offset);

            // for(int read_index = 0; read_index < nchar; read_index++){
            //     ap_uint<8> in = input[input_offset + read_index];
            //     if (read_index < npixel){
            //         inframe[read_index] = in;
            //     }
            //     else{
            //         intag[read_index - npixel] = in;
            //     }
            // }
        
            // process, little endian
            // for(int i = 0; i < entheight; i++){
            //     #pragma HLS unroll
            //     for(int j = 0; j < entwidth; j++){
            //         #pragma HLS unroll
            //         ap_uint<8> pixel = inframe[i * (entwidth + 1) + j];
            //         frame.range(1023 - (i * entwidth + j), 1023 - (i * entwidth + j)) = pixel.range(0,0);
            //     }
            // }

            // for(int i = 0; i < entheight; i++){
            //     #pragma HLS unroll
            //     for(int j = 0; j < entwidth; j++){
            //         #pragma HLS unroll
            //         ap_uint<8> pixel = inframe[i * (entwidth + 1) + j];
            //         int byte_po = j/8;
            //         int bit_po = j - byte_po *8;
            //         int po = byte_po * 8 + 7 - bit_po;
            //         // #ifndef __SYNTHESIS__
            //         //     printf("j: %d, po: %d\n", j, po);
            //         // #endif
            //         frame[i].range(po, po) = pixel.range(0,0);
            //     }
            // }
            

            // tag.range(7, 0) = intag[1];

            // //write
            // int write_offset = loop_index * (entheight + 1);
            // for(int i = 0; i < entheight + 1; i++){
            //     ap_int<32> out;
            //     if (i < entheight){
            //         out = frame[i];
            //         // #ifndef __SYNTHESIS__
            //         //     printf("j: %d, po: %d\n", j, po);
            //         // #endif
            //     }
            //     else{
            //         out = tag;
            //     }
            //     output[write_offset + i] = out;
            // }
                   
        }
       

    }
}