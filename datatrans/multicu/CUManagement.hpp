#include <vector>
#include <iostream>
#include <CL/cl2.hpp>

typedef struct  mCU {

  std::vector<cl::Event> mEvents;
  cl::Event mEvent[3];	
  int      mId;

  mCU(int id);

  void sync();

} mCU;

//void event_cb(cl::Event event, cl_int cmd_status, void *id);

void CL_CALLBACK event_cb(cl_event event, cl_int cmd_status, void *id);