#ifndef _VHL_DATA_HASH_MAP_H_
#define _VHL_DATA_HASH_MAP_H_

#include "../vhl.h"

#define DATA_TYPES_COUNT 10
#define MAX_ENTRIES_PER_BUCKET 32
#define ALLOC_BLOCK_SIZE 8096

//This implementation relies on the assumption that there are no more than MAX_HALF16 types, but for memory reasons, the limit is 256
#define MAX_TYPE_COUNT 256

int data_hmap_initialize(VHLCalls *calls);
int data_hmap_add(VHLCalls *calls, int value, int type);
int data_hmap_remove(VHLCalls *calls, int value);
int data_hmap_clear(VHLCalls *calls);


#endif
