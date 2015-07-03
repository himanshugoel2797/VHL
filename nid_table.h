#ifndef _VHL_NID_TABLE_H_
#define _VHL_NID_TABLE_H_

#include <psp2/types.h>
#include "utils/bithacks.h"
#include "arm_tools.h"
#include "config.h"
#include "elf_headers.h"
#include "utils/utils.h"
#include "vhl.h"
#include "utils/nid_storage.h"
#include "nidcache.h"


#define NID_TABLE_MAX_MODULES 256

#define SCE_MODULE_INFO_EXPECTED_ATTR     0x0000
#define SCE_MODULE_INFO_EXPECTED_VER      0x0101


int nidTable_initialize();
int resolveStub(void *stub, SceNID nid, nidTable_entry *entry);
SceModuleInfo* nidTable_findModuleInfo(void* location, SceUInt size, char* libname);
int nidTable_isValidModuleInfo(SceModuleInfo *m_info);
int nidTable_resolveFromModule(VHLCalls *calls, SceLoadedModuleInfo *target);
int nidTable_resolveImportFromNID(VHLCalls *calls, SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName);
int nidTable_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls);
int nidTable_resolveAll(VHLCalls *calls);
int nidTable_exportFunc(VHLCalls *calls, void *target, SceNID nid);

int nidTable_setNIDaddress(VHLCalls *calls, void *stub, SceNID nid);

#endif
