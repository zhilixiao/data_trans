#include "check.hpp"

std::string get_build_tgt(int *argc, char **argv)
{
	char build_tgt[8]; 
	int arg_idx = *argc - 1;

	std::cerr << "Grab build target from argv" << std::endl;
	std::cerr << "arg_idx = " << arg_idx << std::endl;
	std::cerr << "argv[" << arg_idx << "] = " << argv[arg_idx] << std::endl;
	if (arg_idx > 0)
		int ret = std::sscanf(argv[arg_idx], "%8s", build_tgt);

	std::string ret_build_tgt(build_tgt);
	std::cerr << "Build tgt was found to be " << ret_build_tgt << std::endl;

	*argc -= 1;
	std::cerr << "argc now = " << *argc << std::endl;
	return std::string(build_tgt);
}


int get_krnl_version(int *argc, char **argv)
{
	int v_num = 0;
	int shift = 0;
	int arg_idx = *argc - 1;

	std::cerr << "Grab kernel version number" << std::endl;
	std::cerr << "arg_idx = " << arg_idx << std::endl;
	std::cerr << "argv[" << arg_idx << "] = " << argv[arg_idx] << std::endl;
	// if (arg_idx > 0)
	// {
		int ret = std::sscanf(argv[arg_idx], "%d", &v_num);
		std::cerr << "Using version " << v_num << std::endl;
		if (ret == 1)
			++shift;
	// }
	std::cerr << "Using version " << v_num << std::endl;

	*argc -= shift;
	std::cerr << "argc now = " << *argc << std::endl;
	return v_num;
}

char* read_binary_file(
	const std::string &xclbin_file_name, unsigned &nb) 
{
    std::cout << "INFO: Reading " << xclbin_file_name << std::endl;

	if(access(xclbin_file_name.c_str(), R_OK) != 0) {
		printf("ERROR: %s xclbin not available please build\n", xclbin_file_name.c_str());
		exit(EXIT_FAILURE);
	}
    //Loading XCL Bin into char buffer 
    std::cout << "Loading: '" << xclbin_file_name.c_str() << "'\n";
    std::ifstream bin_file(xclbin_file_name.c_str(), std::ifstream::binary);
    bin_file.seekg (0, bin_file.end);
    nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char *buf = new char [nb];
    bin_file.read(buf, nb);
    return buf;
}

