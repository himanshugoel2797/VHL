#ifndef _VHL_RESOURCE_MANAGER_H_
#define _VHL_RESOURCE_MANAGER_H_

#include "../vhl.h"

int resource_manager_initialize(VHLCalls *calls);

int resource_manager_free_all(int homebrewSlot);  //Frees all system resources associated with a hombrew in the given slot


#endif
