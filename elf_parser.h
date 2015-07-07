#ifndef _VHL_ELF_PARSER_H_
#define _VHL_ELF_PARSER_H_

#include "vhl.h"

#include "elf_headers.h"
#include "exports.h"
#include "nid_table.h"

static inline int align(int x, int n) {
  return (((x >> n) + 1) << n );
}

#define FOUR_KB_ALIGN(x) align(x, 12)
#define MB_ALIGN(x) align(x, 20)

typedef struct {
        void *data_mem_loc;
        SceUID data_mem_uid;
        int data_mem_size;

        void *exec_mem_loc;
        SceUID exec_mem_uid;
        int exec_mem_size;

        void *elf_mem_loc;
        SceUID elf_mem_uid;
        int elf_mem_size;

        int (*entryPoint)(int, char**);
} allocData;


int block_manager_initialize(VHLCalls *calls);
int block_manager_free_old_data(VHLCalls *calls, int curSlot);

int elf_parser_start(VHLCalls *calls, int curSlot);
int elf_parser_load(VHLCalls *calls, int curSlot, const char* file, void** entryPoint);


#endif
