#ifndef _VHL_NID_TABLE_H_
#define _VHL_NID_TABLE_H_

#include "utils/bithacks.h"
#include "utils/utils.h"
#include "utils/nid_storage.h"

#include "vhl.h"
#include "arm_tools.h"
#include "config.h"
#include "module_headers.h"
#include "nidcache.h"


#define NID_TABLE_MAX_MODULES 256

#define SCE_MODULE_INFO_EXPECTED_ATTR     0x0000
#define SCE_MODULE_INFO_EXPECTED_VER      0x0101


int nid_table_initialize();
int nid_table_analyzeStub(void *stub, SceNID nid, nidTable_entry *entry);
SceModuleInfo* nid_table_findModuleInfo(void* location, SceUInt size, char* libname);
int nid_table_isValidModuleInfo(SceModuleInfo *m_info);
int nid_table_resolveFromModule(VHLCalls *calls, SceLoadedModuleInfo *target);
int nid_table_resolveImportFromNID(VHLCalls *calls, SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName);
int nid_table_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls);
int nid_table_resolveAll(VHLCalls *calls);
int nid_table_exportFunc(VHLCalls *calls, void *target, SceNID nid);
int nid_table_resolveStub(VHLCalls *calls, void *stub, SceNID nid);

#endif
