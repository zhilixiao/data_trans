#include "CUManagement.hpp"


mCU::mCU(int id) 
{
    mId = id;
}	


void mCU::sync()
{
  	// Wait until the outputs have been read back
	mEvent[3].wait(); 
	//clWaitForEvents(1, &mEvent[2]);
	//clReleaseEvent(mEvent[0]);
   	//clReleaseEvent(mEvent[1]);
   	//clReleaseEvent(mEvent[2]);	
}

// void event_cb(cl::Event event, cl_int cmd_status, void *id) 
// {
// 	if (getenv("XCL_EMULATION_MODE") != NULL) {
// 	 	std::cout << "  kernel finished processing request " << *(int *)id << std::endl;
// 	}
// }

void CL_CALLBACK event_cb(cl_event event, cl_int cmd_status, void *id){
	if (getenv("XCL_EMULATION_MODE") != NULL) {
	 	std::cout << "  kernel finished processing request " << *(int *)id << std::endl;
	}
}