/*
VHL: Vita Homebrew Loader
Copyright (C) 2015  hgoel0974

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef VHL_NID_TABLE_H
#define VHL_NID_TABLE_H

#include <psp2/kernel/modulemgr.h>
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

enum {
        ANALYZE_STUB_OK,
        ANALYZE_STUB_UNRESOLVED,
        ANALYZE_STUB_INVAL
};

int nid_table_initialize();
int nid_table_analyzeStub(const void *stub, SceNID nid, nidTable_entry *entry);
SceModuleInfo* nid_table_findModuleInfo(void* location, SceUInt size, char* libname);
int nid_table_isValidModuleInfo(SceModuleInfo *m_info);
int nid_table_addStubsInModule(Psp2LoadedModuleInfo *target);
void nid_table_resolveVhlPuts(void *p, const UVL_Context *ctx);
void nid_table_resolveVhlPrimaryImports(void *p, size_t size, const SceModuleInfo *libkernel,
        const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM],
        const UVL_Context *ctx);
void nid_table_resolveVhlSecondaryImports(void *p, size_t size, const SceModuleInfo *libkernel,
        const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM],
        const UVL_Context *ctx);
int nid_table_addNIDCacheToTable(const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM]);
int nid_table_addAllStubs(void);
void nid_table_addAllHooks(void);
int nid_table_resolveStub(void *stub, SceNID nid);

#endif
