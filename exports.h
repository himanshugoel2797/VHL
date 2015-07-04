#ifndef _VHL_EXPORTS_H_
#define _VHL_EXPORTS_H_

#include "vhl.h"
#include "nid_table.h"

typedef enum{
  //TODO get nids from UVL and make sure they match
  ALLOC_CODE_MEM = 0xBCEAB831
} EXPORT_NIDS;


int exports_initialize(VHLCalls *calls);

SceUID AllocCodeMemBlock(int size);

#endif
