#ifndef _VHL_ELF_LOADER_H_
#define _VHL_ELF_LOADER_H_

#include "vhl.h"
#include "nid_table.h"
#include "elf_parser.h"
#include "nids.h"

//Contains functions to parse and load elfs
int loader_initialize(VHLCalls *calls);
int loader_loadHomebrew(const char *str, int slot);
int loader_startHomebrew(int slot);


#endif
