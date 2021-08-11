// #ifdef XLX_RODINIA_HPP
// 	#include "xlx_rodinia.hpp"
// #else
// 	#include <CL/cl2.hpp>
// #endif

#include "check.hpp"
#include "digit.h"
#include "trans_opencl.hpp"

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


int main(int argc, char** argv)
{
	char *inputBuffer;
    char *outDigitBuffer;
    char *outTagBuffer;

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
            &inputBuffer);
	
	//output size in char
	size_t outDigitSize = (entheight) * entwidth * ndigit / 8;
	outDigitBuffer = (char *)AlignedMalloc(sizeof(char) * outDigitSize);
	for (int i = 0; i < outDigitSize; i++){
		outDigitBuffer[i] = 0;
	}

    size_t outTagSize = ndigit;
    outTagBuffer = (char *)AlignedMalloc(sizeof(char) * outTagSize);
    for (int i = 0; i < outTagSize; i++){
		outTagBuffer[i] = 0;
	}



	Trans_OpenCL trancl;
	cl_int err, result;

	trancl.init(version, build_tgt, kernel_name);

	//Allocate global memory, what about stream?		
	cl::Buffer input_d;
	cl::Buffer outDigitBuffer_d;
    cl::Buffer outTagBuffer_d;

	input_d = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
			inputSize * sizeof(char), 
			inputBuffer, //NULL
			&err
		);

	OCL_CHECK_NO_CALL(err, "creating input_d cl::Buffer");

	outDigitBuffer_d = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outDigitSize * sizeof(char), 
			outDigitBuffer, //NULL
			&err
		);

	OCL_CHECK_NO_CALL(err, "creating output_d cl::Buffer");
	

    outTagBuffer_d = cl::Buffer(trancl.ctx,
			CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
			outTagSize * sizeof(char), 
			outTagBuffer, //NULL
			&err
		);
    OCL_CHECK_NO_CALL(err, "creating output_d cl::Buffer");

	//set kernel arguments
	OCL_CHECK(err, err = 
			trancl.str_kernel_map[kernel_name.c_str()].setArg(0, input_d)
		);

	OCL_CHECK(err, err = 
			trancl.str_kernel_map[kernel_name.c_str()].setArg(1, outDigitBuffer_d)
		);

    OCL_CHECK(err, err = 
			trancl.str_kernel_map[kernel_name.c_str()].setArg(2, outTagBuffer_d)
		);

	cout << "kernel arguments are set" << endl;
	
	//enqueue buffers
	// OCL_CHECK(err, err = trancl.cmd_q.enqueueMigrateMemObjects(
	// 			input_d, 1, 0, 
	// 			inputSize * sizeof(char), 
	// 			inputBuffer, //NULL
	// 			NULL, NULL)
	// 		);
	// trancl.cmd_q.finish();


	OCL_CHECK(err, trancl.cmd_q.enqueueMigrateMemObjects(
			{input_d, outDigitBuffer_d, outTagBuffer_d},
			0)
		);

	trancl.cmd_q.finish();
	cout << "finish migrate memobjects" << endl;	


	//lauch kernel
	OCL_CHECK(err, 
				err = trancl.cmd_q.enqueueTask(
						trancl.str_kernel_map[kernel_name.c_str()],
						NULL,
						NULL)
		);
	trancl.cmd_q.finish();

	cout << "finish kernel execution" << endl;

	//read output buffer?
	OCL_CHECK(err, err = trancl.cmd_q.enqueueReadBuffer(outDigitBuffer_d, 1, 0, 
		outDigitSize * sizeof(char), outDigitBuffer, NULL,NULL)
	);

	trancl.cmd_q.finish();

	cout << "finish copying results" << endl;



	write_file(OUTPUT_FILEPATH, &outDigitBuffer, outDigitSize);

	if(inputBuffer)
		free(inputBuffer);

	if(outDigitBuffer)
		free(outDigitBuffer);
	
    if(outTagBuffer)
		free(outTagBuffer);

}