// #ifdef XLX_RODINIA_HPP
// 	#include "xlx_rodinia.hpp"
// #else
// 	#include <CL/cl2.hpp>
// #endif
#define INPUT_FILEPATH    "../input/input_original.txt"
#define OUTPUT_FILEPATH1    "./output/streams-digit-in-bin.txt"
#define OUTPUT_FILEPATH2    "./output/streams-tag-in-bin.txt"

//this header file should be included in check.hpp
//included for cl_mem_ext_ptr_t struct to assign physical memory in host code
#include <CL/cl_ext.h>
//#include <CL/cl_ext_xilinx.h>

#include "../check.hpp"
#include "../digit.h"
#include "trans_opencl.hpp"
//#include ""

#ifdef TIFF_OUT
	#include "tiff_out.hpp"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>

//for simple kernel runtime computation
#include <chrono>
#include <ratio>
#include <iomanip>
#include <ctime>

using std::cout;
using std::endl;

void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s <kernel_version> <build_target>\n", argv[0]);
	exit(1);
}

void* AlignedMalloc(size_t size)
{
	void *temp_ptr;
	if ( posix_memalign (&temp_ptr, 4096, size) )
	{
		fprintf(stderr, "Aligned Malloc failed due to insufficient memory.\n");
		exit(-1);
	}
	return temp_ptr;
}


size_t load_input_buffer(std::string filepath, char **buffer) {
	std::ifstream infile;
	infile.open(filepath, std::fstream::in);
	if (infile.is_open()){
		// find the length of the file
		infile.seekg(0, infile.end);
		size_t fileSize = infile.tellg();
		infile.seekg(0, infile.beg);

		*buffer = (char *)AlignedMalloc(sizeof(char) * fileSize + 1);

		infile.read(*buffer, fileSize);
		cout << "File successfully read" << endl;

		cout << "Filesize in bytes:" << fileSize << endl;
		infile.close();
		return fileSize;
	}
	else{
		cout << "Cannot Open File!" << endl;
		return 0;
	}
	return 0;
}

void write_file(std::string filepath, char **buffer, size_t fileLength){
	std::ofstream ofile;
	ofile.open(filepath, std::ofstream::out); //std::ofstream::binary | std::ofstream::app

	if (ofile.is_open()){
		ofile.write(*buffer, fileLength);
		cout << "File successfully write" << endl;
		ofile.close();
	}
	else{
		cout << "Cannot Open File!" << endl;
	}	
}

void multi_write_file(std::string filepath, char **buffer0, char **buffer1, char **buffer2, char **buffer3, size_t fileLength){
	std::ofstream ofile;
	ofile.open(filepath, std::ofstream::out); //std::ofstream::binary | std::ofstream::app

	if (ofile.is_open()){
		ofile.write(*buffer0, fileLength);
		ofile.write(*buffer1, fileLength);
		ofile.write(*buffer2, fileLength);
		ofile.write(*buffer3, fileLength);
		cout << "File successfully write" << endl;
		ofile.close();
	}
	else{
		cout << "Cannot Open File!" << endl;
	}	
}


/*
multiple cu:
1. Single context, device and program are needed
	Context created from device
	Program are created from bin files and device

2. Single command queue is needed


2. Kernel dispatcher has:
	cl::Context 
	cl::Kernel created from program 
	cl::CommandQueue 
	cl::Buffer 
	CU counter

*/
int main(int argc, char** argv)
{
	char *inputBuffer0, *inputBuffer1, *inputBuffer2, *inputBuffer3;
	
    char *outputBuffer0, *outputBuffer1, *outputBuffer2, *outputBuffer3;
	char *outTagBuffer0, *outTagBuffer1, *outTagBuffer2, *outTagBuffer3;

	if (argc != 3){
		usage(argc, argv);
	}

	std::string build_tgt;
	std::string kernel_name("digit_kernel");

	int version;
	build_tgt = get_build_tgt(&argc, argv);
	version = get_krnl_version(&argc, argv);

	cout << "version = " << version << endl;
	cout << "build_tgt = " << build_tgt << endl;

	//should be changed to stream!
	size_t inputSize = load_input_buffer(INPUT_FILEPATH,
            &inputBuffer0);

	load_input_buffer(INPUT_FILEPATH, &inputBuffer1);

	load_input_buffer(INPUT_FILEPATH, &inputBuffer2);

	load_input_buffer(INPUT_FILEPATH, &inputBuffer3);
	
	//output size in char
	size_t outputSize = (entheight) * entwidth * ndigit / 8;
	outputBuffer0 = (char *)AlignedMalloc(sizeof(char) * outputSize);
	outputBuffer1 = (char *)AlignedMalloc(sizeof(char) * outputSize);
	outputBuffer2 = (char *)AlignedMalloc(sizeof(char) * outputSize);
	outputBuffer3 = (char *)AlignedMalloc(sizeof(char) * outputSize);

	for (int i = 0; i < outputSize; i++){
		outputBuffer0[i] = 0;
		outputBuffer1[i] = 0;
		outputBuffer2[i] = 0;
		outputBuffer3[i] = 0;
	}

	size_t outTagSize = ndigit;
    outTagBuffer0 = (char *)AlignedMalloc(sizeof(char) * outTagSize);
	outTagBuffer1 = (char *)AlignedMalloc(sizeof(char) * outTagSize);
	outTagBuffer2 = (char *)AlignedMalloc(sizeof(char) * outTagSize);
	outTagBuffer3 = (char *)AlignedMalloc(sizeof(char) * outTagSize);
    for (int i = 0; i < outTagSize; i++){
		outTagBuffer0[i] = 0;
		outTagBuffer1[i] = 0;
		outTagBuffer2[i] = 0;
		outTagBuffer3[i] = 0;
	}


	Trans_OpenCL trancl;
	cl_int err, result;

	trancl.init(version, build_tgt, kernel_name);

	std::vector<cl_mem_ext_ptr_t> cu0_ext(3);
	std::vector<cl_mem_ext_ptr_t> cu1_ext(3);
	cu0_ext[0].flags = 0|XCL_MEM_TOPOLOGY;
    cu0_ext[0].obj = 0; // pagerank2;
    cu0_ext[0].param = 0;
	cu0_ext[1].flags = 1|XCL_MEM_TOPOLOGY;
    cu0_ext[1].obj = outputBuffer0; // pagerank2;|XCL_MEM_EXT_HOST_ONLY
    cu0_ext[1].param = 0;
	cu0_ext[2].flags = 4|XCL_MEM_TOPOLOGY;
    cu0_ext[2].obj = outTagBuffer0; // pagerank2;
    cu0_ext[2].param = 0;

	cu1_ext[0].flags = XCL_MEM_DDR_BANK2;
    cu1_ext[0].obj = inputBuffer1; // pagerank2;
    cu1_ext[0].param = 0;
	cu1_ext[1].flags = 3|XCL_MEM_TOPOLOGY;
    cu1_ext[1].obj = outputBuffer1; // pagerank2;
    cu1_ext[1].param = 0;
	cu1_ext[2].flags = 5|XCL_MEM_TOPOLOGY;
    cu1_ext[2].obj = outTagBuffer1; // pagerank2;
    cu1_ext[2].param = 0;



	//Allocate global memory, what about stream?		
	cl::Buffer input_d0, input_d1, input_d2, input_d3;
	cl::Buffer output_d0, output_d1, output_d2, output_d3;
	cl::Buffer tag_d0, tag_d1, tag_d2, tag_d3;

	input_d0 = cl::Buffer(trancl.ctx,
			CL_MEM_READ_ONLY  | CL_MEM_EXT_PTR_XILINX,
			inputSize * sizeof(char), 
			&cu0_ext[0], //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating input_d0 cl::Buffer");

	input_d1 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY , //CL_MEM_USE_HOST_PTR | | CL_MEM_EXT_PTR_XILINX| CL_MEM_EXT_PTR_XILINX
			inputSize * sizeof(char), 
			inputBuffer1, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating input_d1 cl::Buffer");

	input_d2 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
			inputSize * sizeof(char), 
			inputBuffer2, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating input_d2 cl::Buffer");

	input_d3 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
			inputSize * sizeof(char), 
			inputBuffer3, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating input_d3 cl::Buffer");




	output_d0 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outputSize  * sizeof(char), 
			outputBuffer0, //NULL
			&err
		);

	OCL_CHECK_NO_CALL(err, "creating output_d0 cl::Buffer");


	output_d1 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outputSize  * sizeof(char), 
			outputBuffer1 , //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating output_d1 cl::Buffer");


	output_d2 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outputSize * sizeof(char), 
			outputBuffer2 , //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating output_d2 cl::Buffer");

	output_d3 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outputSize  * sizeof(char), 
			outputBuffer3, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating output_d3 cl::Buffer");
	
	//cout << "buffers are created" << endl;


	tag_d0 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			ndigit  * sizeof(char), 
			outTagBuffer0, //NULL
			&err
		);

	OCL_CHECK_NO_CALL(err, "creating output_d0 cl::Buffer");


	tag_d1 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			ndigit  * sizeof(char), 
			outTagBuffer1, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating output_d1 cl::Buffer");


	tag_d2 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			ndigit * sizeof(char), 
			outTagBuffer2, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating tag_d2 cl::Buffer");

	tag_d3 = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			ndigit * sizeof(char), 
			outTagBuffer3, //NULL
			&err
		);
	OCL_CHECK_NO_CALL(err, "creating tag_d3 cl::Buffer");
	
	cout << "buffers are created" << endl;
	//set kernel arguments
	// OCL_CHECK(err, err = 
	// 		trancl.str_kernel_map[kernel_name.c_str()].setArg(0, input_d)
	// 	);

	// OCL_CHECK(err, err = 
	// 		trancl.str_kernel_map[kernel_name.c_str()].setArg(1, output_d)
	// 	);

	// OCL_CHECK(err, err = 
	// 		trancl.str_kernel_map[kernel_name.c_str()].setArg(2, 1)
	// 	);

	// cout << "kernel arguments are set" << endl;
	
	//enqueue buffers
	// OCL_CHECK(err, err = trancl.cmd_q.enqueueMigrateMemObjects(
	// 			input_d, 1, 0, 
	// 			inputSize * sizeof(char), 
	// 			inputBuffer, //NULL
	// 			NULL, NULL)
	// 		);
	// trancl.cmd_q.finish();


	// OCL_CHECK(err, trancl.cmd_q.enqueueMigrateMemObjects(
	// 		{input_d, output_d},
	// 		0)
	// 	);

	// trancl.cmd_q.finish();
	// cout << "finish migrate memobjects" << endl;	


	// //lauch kernel
	// OCL_CHECK(err, 
	// 			err = trancl.cmd_q.enqueueTask(
	// 					trancl.str_kernel_map[kernel_name.c_str()],
	// 					NULL,
	// 					NULL)
	// 	);
	// trancl.cmd_q.finish();

	// cout << "finish kernel execution" << endl;

	// //read output buffer?
	// OCL_CHECK(err, err = trancl.cmd_q.enqueueReadBuffer(output_d, 1, 0, 
	// 	outputSize * sizeof(char), outputBuffer, NULL,NULL)
	// );

	// trancl.cmd_q.finish();

	// cout << "finish copying results" << endl;


	mCU* cu0 = trancl.createCU(0, &input_d0, &output_d0, &tag_d0, ndigit);
	mCU* cu1 = trancl.createCU(1, &input_d1, &output_d1, &tag_d1, ndigit);
	//mCU* cu2 = trancl.createCU(2, &input_d2, &output_d2, &tag_d2, ndigit);
	//mCU* cu3 = trancl.createCU(3, &input_d3, &output_d3, &tag_d3, ndigit);

	cu0->sync();
	cu1->sync();
	//cu2->sync();
	//cu3->sync();

	std::cout << "finish copying results" << std::endl;

	multi_write_file(OUTPUT_FILEPATH1, &outputBuffer0, &outputBuffer1, &outputBuffer2, &outputBuffer3, outputSize);

	multi_write_file(OUTPUT_FILEPATH2, &outTagBuffer0, &outTagBuffer1, &outTagBuffer2, &outTagBuffer3, ndigit);

	if(inputBuffer0)
		free(inputBuffer0);
	if(inputBuffer1)
		free(inputBuffer1);
	if(inputBuffer2)
		free(inputBuffer2);
	if(inputBuffer3)
		free(inputBuffer3);

	if(outputBuffer0)
		free(outputBuffer0);
	if(outputBuffer1)
		free(outputBuffer1);
	if(outputBuffer2)
		free(outputBuffer2);
	if(outputBuffer3)
		free(outputBuffer3);
	
	if(outTagBuffer0)
		free(outTagBuffer0);
	if(outTagBuffer1)
		free(outTagBuffer1);
	if(outTagBuffer2)
		free(outTagBuffer2);
	if(outTagBuffer3)
		free(outTagBuffer3);

}