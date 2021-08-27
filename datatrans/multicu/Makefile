CXX=g++
CXXFLGAS := -std=c++11

HOST_SRC = multicu_host.cpp ../check.cpp trans_opencl.cpp CUManagement.cpp
HOST_EXE = multicu_host

INC_DIR = ${XILINX_XRT}/include/
INC_FLGAS = -I$(INC_DIR)

LIB_DIR = ${XILINX_XRT}/lib/
LIBS = -lOpenCL -lpthread -lrt -lstdc++
LDFLAGS = -L$(LIB_DIR) $(LIBS)

HWC ?= v++ -O3
HW_TARGET ?= hw
KERNEL_NAME ?= digit_kernel
KERNEL_SRC ?= stream_kernel_v1.cpp
#port_widen.cpp
#kernel_littel_endian_v1.cpp

# Required flags
DESIGN_CFG ?= design.cfg
TCL_FILE ?= runPre.tcl
HWCFLAGS := --config $(DESIGN_CFG) --hls.pre_tcl $(TCL_FILE)
HWCFLAGS += -t ${HW_TARGET}

$(info HWCFLAGS=$(HWCFLAGS))

#HW stuff
XO_FILE ?= stream_kernel_v1.xo
XCLBIN_FILE ?= stream_kernel_v1.xclbin


default: all

all: $(host) 

host: $(HOST_SRC)
	$(CXX) $(CXXFLGAS) $(HOST_SRC) $(INC_FLGAS) $(LDFLAGS) -o $@


$(XO_FILE): $(KERNEL_SRC)
	$(HWC) -c $(HWCFLAGS) -k $(KERNEL_NAME) -I ./ $(KERNEL_SRC) -o $@

$(XCLBIN_FILE): $(KERNEL_SRC)
	$(HWC) -l $(HWCFLAGS) $(XO_FILE) -o $@	

bin: $(XO_FILE) $(XCLBIN_FILE)
    
cleanbin:
	rm *.xo *.xclbin* *.log

cleansum:
	rm *summary


#g++ -std=c++11 \
nw_main_xilinx.cpp ../common_xlx/xlx_rodinia.cpp nw.cpp nw_opencl.cpp \
-I../common_xlx -I${XILINX_XRT}/include/ \
-L${XILINX_XRT}/lib/ -lOpenCL -lpthread -lrt -lstdc++