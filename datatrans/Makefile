CXX=g++
CXXFLGAS := -std=c++11

HOST_SRC = digit_host.cpp check.cpp trans_opencl.cpp
HOST_EXE = host

INC_DIR = ${XILINX_XRT}/include/
INC_FLGAS = -I$(INC_DIR)

LIB_DIR = ${XILINX_XRT}/lib/
LIBS = -lOpenCL -lpthread -lrt -lstdc++
LDFLAGS = -L$(LIB_DIR) $(LIBS)

HWC ?= v++ -O3
HW_TARGET ?= sw_emu
KERNEL_NAME ?= digit_kernel
KERNEL_SRC ?= kernel_apint.cpp

# Required flags
DESIGN_CFG ?= design.cfg
HWCFLAGS := --config $(DESIGN_CFG)
HWCFLAGS += -t ${HW_TARGET}

$(info HWCFLAGS=$(HWCFLAGS))

#HW stuff
XO_FILE ?= kernel_v0.xo
XCLBIN_FILE ?= kernel_v0.xclbin


default: all

all: $(host) 

host: $(HOST_SRC)
	$(CXX) $(CXXFLGAS) $(HOST_SRC) $(INC_FLGAS) $(LDFLAGS) -o $@


$(XO_FILE): $(KERNEL_SRC)
	$(HWC) -c $(HWCFLAGS) -k $(KERNEL_NAME) -I ./ $(KERNEL_SRC) -o $@

$(XCLBIN_FILE): $(KERNEL_SRC)
	$(HWC) -l $(HWCFLAGS) $(XO_FILE) -o $@	

bin: $(XO_FILE) $(XCLBIN_FILE)
    
clean:
	rm host *.log



#g++ -std=c++11 \
nw_main_xilinx.cpp ../common_xlx/xlx_rodinia.cpp nw.cpp nw_opencl.cpp \
-I../common_xlx -I${XILINX_XRT}/include/ \
-L${XILINX_XRT}/lib/ -lOpenCL -lpthread -lrt -lstdc++