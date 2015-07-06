#ifndef _VHL_EXPORTS_H_
#define _VHL_EXPORTS_H_

#include "vhl.h"
#include "nid_table.h"
#include "elf_parser.h"

typedef enum {
        //TODO get nids from UVL and make sure they match
        ALLOC_CODE_MEM = 0xBCEAB831,
        PRINTF = 0x9A004680,
        PUTS = 1506009457,
        UNLOCK = 0x98D1C91D,
        LOCK = 0xEEC99826,
        FLUSH = 0xC85B400C,
        LOG = 0xD4F59028,
        LOAD_ELF = 1,
        START_ELF = 2
} EXPORT_NIDS;


int exports_initialize(VHLCalls *calls);

SceUID AllocCodeMemBlock(int size);

#endif
