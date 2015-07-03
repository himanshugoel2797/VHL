#ifndef _VHL_MEM_MGR_H_
#define _VHL_MEM_MGR_H_

#include "../vhl.h"

#define MEMORY_ALLOC_SIZE 8 * 1024 * 1024    //Allocate 8 MB worth of executable memory to manage

int memory_manager_initialize(VHLCalls *calls); 

#endif
