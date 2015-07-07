#ifndef _VHL_EXPORTS_H_
#define _VHL_EXPORTS_H_

#include "vhl.h"
#include "nid_table.h"
#include "elf_parser.h"
#include "nids.h"

int exports_initialize(VHLCalls *calls);

SceUID AllocCodeMemBlock(int size);

#endif
