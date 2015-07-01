#ifndef _VHL_NID_TABLE_H_
#define _VHL_NID_TABLE_H_

#include <psp2/types.h>
#include "bithacks.h"
#include "arm_tools.h"
#include "config.h"
#include "elf_headers.h"
#include "utils.h"
#include "vhl.h"
#include "nidcache.h"

#define NID_TABLE_MAX_ENTRIES 16384
#define NID_TABLE_MAX_MODULES 256
#define NID_TABLE_CACHE_FILE VHL_DATA_PATH "/nidCache.bin"

#define SCE_MODULE_INFO_EXPECTED_ATTR     0x0000
#define SCE_MODULE_INFO_EXPECTED_VER      0x0101

typedef enum  {
        ENTRY_TYPES_UNKN = 0,
        ENTRY_TYPES_FUNCTION,
        ENTRY_TYPES_SYSCALL,
        ENTRY_TYPES_VARIABLE
}EntryTypes;

//Represents an entry in the NID table
typedef struct {
        SceNID nid;
        int type;
        union {
                void *function;
                SceUInt location;
        } value;
} nidTable_entry;

int nidTable_initialize();
int resolveStub(void *stub, SceNID nid, nidTable_entry *entry);
SceModuleInfo* nidTable_findModuleInfo(void* location, SceUInt size, char* libname);
int nidTable_isValidModuleInfo(SceModuleInfo *m_info);
int nidTable_resolveImportFromNID(VHLCalls *calls, SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName);
int nidTable_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls);
int nidTable_resolveAll(VHLCalls *calls);
int nidTable_exportFunc(void *target, SceNID nid);

#endif
