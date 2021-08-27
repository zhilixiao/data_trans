#include "trans_opencl.hpp"



//standard constructor
Trans_OpenCL::Trans_OpenCL(
	int kernel_v,
	bool print_debug				
) : kernel_v(kernel_v), verbose_flag(print_debug)
{
}

void Trans_OpenCL::GetDevices()
{
	cl_int err;
	std::vector<cl::Platform> platforms;
	OCL_CHECK(err, err = cl::Platform::get(&platforms));
	cl::Platform platform;
	for (int i  = 0 ; i < platforms.size(); i++){
		platform = platforms[i];
		OCL_CHECK(err, std::string platformName = platform.getInfo<CL_PLATFORM_NAME>(&err));
		if (platformName == "Xilinx"){
			std::cout << "Found Platform" << std::endl;
			std::cout << "Platform Name: " << platformName.c_str() << std::endl;
			break;
		}
	}

	OCL_CHECK(err, platform.getDevices(CL_DEVICE_TYPE_ALL, &devices));
}

void Trans_OpenCL::ReadBinaries(int version, std::string binary_name,
	cl::Program::Binaries& bins)
{
	unsigned int file_buf_size;	
	char *file_buf = read_binary_file(binary_name, file_buf_size);
	bins = {{file_buf, file_buf_size}};

	// TODO: figure out a way to free file_buf

}

//1. Define Plaform Device Context and Queues
//2. Create & build the program
//3. Create and setup kernel, creation of memory objects like buffer are left out in main()
void Trans_OpenCL::init(int version, const std::string build_tgt, 
	const std::string kernel_name)
{
	cl_int err;
	//1. Step 1
    GetDevices();

	devices.resize(1); 
	// OCL_CHECK(err, cl::Device device(devices[0]));

	ctx = cl::Context(devices[0], NULL, NULL, NULL, &err);
	OCL_CHECK_NO_CALL(err, "creating ctx");

	std::cout << "hi starting cmd q" << std::endl;
	// out-of-order commmand queue
	cmd_q = cl::CommandQueue(ctx, devices[0], CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
	OCL_CHECK_NO_CALL(err, "creating cmd_q");


	//3 & 4.Step4 
	// need to change the kernel string in the future
	std::string my_kernel_name(
		std::string("./") + build_tgt + std::string("/stream_kernel_v")
		+ std::to_string(version)
	);
	//binaries is the xlinix bin file
	std::string binary_name(
		my_kernel_name
		+ std::string(".xclbin")
	);
	std::cout << "my_kernel_name = " << my_kernel_name << std::endl;
	std::cout << "binary_name = " << binary_name << std::endl;

	cl::Program::Binaries bins;

	ReadBinaries(version, binary_name, bins);

	OCL_CHECK(err, cl::Program program(ctx, devices, bins, NULL, &err)); 

	//OCL_CHECK(err, cl::Kernel krnl_dynproc(program, kernel_name.c_str(), &err));

	OCL_CHECK(err, cl::Kernel krnl_0(program, "digit_kernel:{digit_kernel_0}", &err));

	OCL_CHECK(err, cl::Kernel krnl_1(program, "digit_kernel:{digit_kernel_1}", &err));

	mkernel[0] = krnl_0;
	mkernel[1] = krnl_1;

/*
  TODO:
	// Memory cleanup for the variable used to hold the kernel source.
*/
    //free binaries file buffer
	for (int i = 0; i < bins.size(); ++i)
		delete[] const_cast<char *>(static_cast<const char*>(bins[i].first));
}


Trans_OpenCL::~Trans_OpenCL()
{
	
}

//, int outputSize, char *output, char *tag
mCU* Trans_OpenCL::createCU(int cuID, cl::Buffer *input_d, cl::Buffer *output_d, cl::Buffer *tag_d, int total, 
								char *inputBuffer, char *outputBuffer, char *outTagBuffer)
{
	cl_int err;
	mCU*  req = new mCU(cuID);
	std::cout << "create CU " << cuID << std::endl;
	//create buffer

	//set kernel argument
	OCL_CHECK(err, err = 
			mkernel[cuID].setArg(0, *input_d)
		);

	OCL_CHECK(err, err = 
			mkernel[cuID].setArg(1, *output_d)
		);

	OCL_CHECK(err, err = 
			mkernel[cuID].setArg(2, *tag_d)
		);

	OCL_CHECK(err, err = 
			mkernel[cuID].setArg(3, total)
		);
	//cout << "kernel arguments are set" << endl;

	//write input buffer to device
	OCL_CHECK(err, cmd_q.enqueueMigrateMemObjects(
			{*input_d}, //, output_d
			0, NULL, &req->mEvent[0])
		);


	size_t inputSize = total * 1059;
	OCL_CHECK(err, cmd_q.enqueueWriteBuffer(
										*input_d, CL_FALSE, //, output_d
										0, inputSize, inputBuffer, 
										NULL, &req->mEvent[0])
		);


	req->mEvents.push_back(req->mEvent[0]);
	//trancl.cmd_q.finish();
	//cout << "migrate input memobjects" << endl;

	//Enqueue Task (schedule execution)
	OCL_CHECK(err, 
				err = cmd_q.enqueueTask(
					mkernel[cuID],
					&req->mEvents,
					&req->mEvent[1])
		);
	//trancl.cmd_q.finish();
	//cout << "kernel execution" << endl;

	req->mEvents.push_back(req->mEvent[1]);

	//read output buffer
	// OCL_CHECK(err, cmd_q.enqueueReadBuffer(
	// 		*output_d,  1, 0, 
	// 		total
	// 		&req->mEvents, &req->mEvent[2])
	// 	);
	// req->mEvents.push_back(req->mEvent[2]);


	// OCL_CHECK(err, cmd_q.enqueueReadBuffer(
	// 		*tag_d,  1, //output_d
	// 		0, &req->mEvents, &req->mEvent[3])
	// 	);
	// req->mEvents.push_back(req->mEvent[3]);
	//clEnqueueMigrateMemObjects(mkernel, 1, *output_d, CL_MIGRATE_MEM_OBJECT_HOST, 1, &req->mEvent[1], &req->mEvent[2]);
	// OCL_CHECK(err, cmd_q.enqueueMigrateMemObjects(
	// 		{*output_d,  *tag_d}, //, output_d
	// 		CL_MIGRATE_MEM_OBJECT_HOST, &req->mEvents, &req->mEvent[2])
	// 	);

	size_t outputSize = 32 * 32 * total / 8;
	OCL_CHECK(err, cmd_q.enqueueReadBuffer(
										*output_d, CL_FALSE, //, output_d
										0, outputSize, outputBuffer, 
										&req->mEvents, &req->mEvent[2])
		);

	req->mEvents.push_back(req->mEvent[2]);

	//size_t inputSize = total * 1059;
	OCL_CHECK(err, cmd_q.enqueueReadBuffer(
										*tag_d, CL_FALSE, //, output_d
										0, total, outTagBuffer, 
										&req->mEvents, &req->mEvent[3])
		);

	req->mEvents.push_back(req->mEvent[3]);

	//Register call back to notify kernel completion

	//req->mEvent[1].setCallback(CL_COMPLETE, &event_cb, &req->mId);
	//clSetEventCallback(req->mEvent[1], CL_COMPLETE, event_cb, &req->mId); 
	return req;
}

