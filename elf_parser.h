#ifndef _VHL_ELF_PARSER_H_
#define _VHL_ELF_PARSER_H_

#include "vhl.h"
#include <psp2/io/fcntl.h>
#include "elf_headers.h"
#include "nid_table.h"

int elfParser_Load(VHLCalls *calls, const char* file, void** entryPoint);


#endif
