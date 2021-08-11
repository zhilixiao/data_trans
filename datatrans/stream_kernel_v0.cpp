#include "digit.h"
#include <ap_int.h>
#include <hls_stream.h>

        //+1 for line delimiter
        // int npixel = entheight * (entwidth + 1);
        // int nchar = npixel + 3;
        // int nbit = nchar * 8;



static void read_input(ap_uint<8> *input, hls::stream<ap_uint<8>> &inStream, int offset) {
    // #ifndef __SYNTHESIS__
    //     printf("read_input\n");
    // #endif 
    for (int i = 0; i < nchar; i++) {
       #pragma HLS PIPELINE II=1
       //#pragma HLS LOOP_TRIPCOUNT min=nchar max=nchar
        //Blocking write command to inStream
        inStream << input[offset + i];
    }
}

static void truncate(hls::stream<ap_uint<8>> &inStream,
                  hls::stream<ap_uint<32>> &digitOutStream,
                  //ap_uint<8>* tagOutBuffer
                  hls::stream<ap_uint<8>> &tagOutStream
                ) {

               
    ap_uint<32> d; 
    for(int i = 0; i < 32; i++){
        #pragma HLS PIPELINE II=1
        for(int j = 0; j < 33; j++){
            ap_uint<8> pixel = inStream.read();
            if (j < 32) 
                d.range(31-j, 31-j) = pixel.range(0,0);
            else
                digitOutStream << d;
        }

    } 
    //maybe need to all three to tagOutStream
    inStream.read();
    ap_uint<8> tag = inStream.read();
    tagOutStream << tag;
    inStream.read();
}



//offset is the number of digits has been processed
static void write_to_onchip_memory(hls::stream<ap_uint<32>> &digitOutStream, hls::stream<ap_uint<8>> &tagStream, 
                                    ap_uint<32> *pixelOutBuffer, ap_uint<8> *tagOutBuffer, int offset) 
{
    // #ifndef __SYNTHESIS__
    //     printf("write_to_onchip_memory\n");
    // #endif 
onmem_wr:
    for (int i = 0; i < 32; i++) {
       #pragma HLS PIPELINE 
       
        //Blocking read command from OutStream
        pixelOutBuffer[offset * 32 + i] = digitOutStream.read();
    }
    tagOutBuffer[offset] = tagStream.read();
}



static void write_to_external_memory(ap_uint<32> *pixelOutBuffer, ap_uint<8> *tagOutBuffer, ap_uint<32> *output, ap_uint<8> *tag){
    // #ifndef __SYNTHESIS__
    //     printf("write_to_external_memory\n");
    // #endif 
mem_wr1:
    for (int i = 0; i < ndigit * 32; i++){
         #pragma HLS PIPELINE 
       output[i] = pixelOutBuffer[i];
    }
mem_wr2:
    for (int i = 0; i < ndigit; i++){
        #pragma HLS PIPELINE 
        tag[i] = tagOutBuffer[i];
    }
}

extern "C"{
    void digit_kernel(ap_uint<8>    *input,
                      ap_uint<32>    *output,
                      ap_uint<8>     *tag
                    )
    {
        #pragma HLS INTERFACE m_axi port=input bundle=gmem 

        #pragma HLS INTERFACE m_axi port=output bundle=gmem1 

        #pragma HLS INTERFACE m_axi port=tag bundle=gmem2 

        //may change to 32 by 32 2D array to reduce resource usage, + 1 for tag and line breaker



        
        static ap_uint<32> pixelOutBuffer[ndigit * 32];
        static ap_uint<8> tagOutBuffer[ndigit];
        // ap_uint<32> frame[32];
        //static ap_uint<8> tag[ndigit];
        static hls::stream<ap_uint<8>> instream;
        static hls::stream<ap_uint<32>> digitOutStream;
        static hls::stream<ap_uint<8>> tagStream;
       
        //v1 could be eliminating the offset to continously streaming the input
        for (int loop_index = 0; loop_index < ndigit; loop_index++){
            
            #pragma HLS DATAFLOW 
            //#pragma HLS dependence intag type=inter dependent=false
            //#pragma HLS dependence inframe type=inter dependent=false
            #ifndef __SYNTHESIS__
                    printf("loop_index: %d\n", loop_index);
            #endif
            int input_offset = nchar * loop_index;
            
            read_input(input, instream, input_offset);
            truncate(instream, digitOutStream, tagStream);
            write_to_onchip_memory(digitOutStream, tagStream, pixelOutBuffer, tagOutBuffer, loop_index);
  
        }
        write_to_external_memory(pixelOutBuffer, tagOutBuffer, output, tag);

    }
}