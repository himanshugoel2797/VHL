#ifndef _VHL_ELF_PARSER_H_
#define _VHL_ELF_PARSER_H_

#include "vhl.h"
#include <psp2/io/fcntl.h>
#include "elf_headers.h"
#include "nid_table.h"

typedef uint16_t Elf32_Half;	// Unsigned half int
typedef uint32_t Elf32_Off;	// Unsigned offset
typedef uint32_t Elf32_Addr;	// Unsigned address
typedef uint32_t Elf32_Word;	// Unsigned int
typedef int32_t  Elf32_Sword;	// Signed int


int elfParser_Load(VHLCalls *calls, const char* file, void** entryPoint);


#endif
