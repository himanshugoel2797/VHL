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
int nid_table_resolveStub(VHLCalls *calls, int priority, void *stub, SceNID nid);
int nid_table_registerHook(VHLCalls *calls, void *func, SceNID nid, void **target);

#endif
