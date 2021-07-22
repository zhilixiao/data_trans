// these pound defines necessary to feed to cl2.hpp
#define CL_HPP_CL_2_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl2.hpp>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h> // for R_OK, access N
// from Xilinx example code
//OCL_CHECK doesn't work if call has templatized function call
#define OCL_CHECK(error,call)                                       \
    call;                                                           \
    if (error != CL_SUCCESS) {                                      \
      printf("%s:%d Error calling " #call ", error code is: %d\n",  \
              __FILE__,__LINE__, error);                            \
      exit(EXIT_FAILURE);                                           \
    }

// Something I had to implement that is a variation of the above macro
#define OCL_CHECK_NO_CALL(error,call)                               \
    if (error != CL_SUCCESS) {                                      \
      printf("%s:%d Error calling " #call ", error code is: %d\n",  \
              __FILE__,__LINE__, error);                            \
      exit(EXIT_FAILURE);                                           \
    }

void display_device_info(std::vector<cl::Platform> &platforms);


std::string get_build_tgt(int *argc, char **argv);	

int get_krnl_version(int *argc, char **argv);

char* read_binary_file(const std::string &xclbin_file_name, unsigned &nb); 