#include <map>
#include <vector>
#include <string>

#include "../check.hpp"
//#include "./ckernel/nw_common.h"
#include <CL/cl2.hpp> // c++ bindings in /usr/include/CL
#include <iostream>
#include "CUManagement.hpp"

// Modifying the OpenCL class from OpenCL.hpp 
// - variable/fxn name case
// - struct instead of a class
// - everything is public. why should it be private?
typedef struct Trans_OpenCL
{
/***********Original local varibles and varibles in initliaze() in nw.c**************/
	//from initialize() & local varibles
	int kernel_v;
	bool verbose_flag;
	size_t size;

	//C++ class for C's data struct
	cl::Context ctx;
	cl::CommandQueue cmd_q;
	std::vector<cl::Device> devices;
	cl::Kernel mkernel;
    //std::map<std::string, cl::Kernel> str_kernel_map; 
    cl::Program program;
    //cl::Kernel kernel(std::string kernelName);

	//constructor
	Trans_OpenCL(){}
	Trans_OpenCL(
		int kernel_v,
		bool print_debug			
	);
	//destructor
	~Trans_OpenCL();
	void GetDevices();
	void BuildKernel(int version, std::string binary_name, 
		cl::Program::Binaries& bins);
	void init(int version, const std::string build_tgt,
		const std::string kernel_name);
	void createKernel(std::string kernelName);

	mCU* createCU(int cuID, cl::Buffer *input_d, cl::Buffer *output_d, cl::Buffer *tag_d, int total);


	
} Trans_OpenCL;
