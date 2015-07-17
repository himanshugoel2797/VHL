/*
nid_table.c : Resolves NIDs and bootstraps the loader
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
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <stdio.h>
#include "hooks.c"
#include "nid_table.h"

static void resolveStubWithBranch(void *stub, const void *loc)
{
        ARM_INSTRUCTION movt;
        ARM_INSTRUCTION movw;
        ARM_INSTRUCTION jmp;

        movw.condition = ARM_CONDITION_ALWAYS;
        movw.type = ARM_MOV_INSTRUCTION;
        movw.instruction = ARM_INST_MOVW;
        movw.argCount = 2;
        movw.value[0] = ARM_R12;
        movw.value[1] = (SceUInt16)((SceUInt)loc & 0xFFFF);

        movt.condition = ARM_CONDITION_ALWAYS;
        movt.type = ARM_MOV_INSTRUCTION;
        movt.instruction = ARM_INST_MOVT;
        movt.argCount = 2;
        movt.value[0] = ARM_R12;
        movt.value[1] = (SceUInt16)((SceUInt)loc >> 16);                          //Only the top part

        jmp.condition = ARM_CONDITION_ALWAYS;
        jmp.type = ARM_BRANCH_INSTRUCTION;
        jmp.instruction = ARM_INST_BX;
        jmp.argCount = 1;
        jmp.value[0] = ARM_R12;

        Assemble(&movw, &((SceUInt*)stub)[0]);
        Assemble(&movt, &((SceUInt*)stub)[1]);
        Assemble(&jmp, &((SceUInt*)stub)[2]);
}

static void resolveStubWithSvc(void *stub, SceUInt n)
{
        ARM_INSTRUCTION movw;
        ARM_INSTRUCTION svc;
        ARM_INSTRUCTION jmp;

        movw.condition = ARM_CONDITION_ALWAYS;
        movw.type = ARM_MOV_INSTRUCTION;
        movw.instruction = ARM_INST_MOVW;
        movw.argCount = 2;
        movw.value[0] = ARM_R12;
        movw.value[1] = (SceUInt16)n;

        svc.condition = ARM_CONDITION_ALWAYS;
        svc.type = ARM_SVC_INSTRUCTION;
        svc.instruction = ARM_INST_SVC;
        svc.argCount = 1;
        svc.value[0] = 0;

        jmp.condition = ARM_CONDITION_ALWAYS;
        jmp.type = ARM_BRANCH_INSTRUCTION;
        jmp.instruction = ARM_INST_BX;
        jmp.argCount = 1;
        jmp.value[0] = ARM_R14;

        Assemble(&movw, &((SceUInt*)stub)[0]);
        Assemble(&svc, &((SceUInt*)stub)[1]);
        Assemble(&jmp, &((SceUInt*)stub)[2]);
}

static int resolveStubWithEntry(void *stub, const nidTable_entry *entry)
{
       switch (entry->type) {
                case ENTRY_TYPES_FUNCTION:
                        resolveStubWithBranch(stub, entry->value.p);
                        break;

                case ENTRY_TYPES_SYSCALL:
                        resolveStubWithSvc(stub, entry->value.i);
                        break;

                case ENTRY_TYPES_VARIABLE:
                        *(SceUInt*)stub = entry->value.i;
                        break;

                default:
                        return -1;
       }

        return 0;
}

__attribute__((hot))
int nid_table_analyzeStub(const void *stub, SceNID nid, nidTable_entry *entry)
{
        entry->nid = nid;
        entry->value.i = 0;

        ARM_INSTRUCTION instr;

        while(1)
        {
                if(Disassemble(stub, &instr) < 0)
                        return ANALYZE_STUB_INVAL;

                switch(instr.instruction)
                {
                        case ARM_INST_MOVW:
                                entry->value.i = instr.value[1];
                                break;
                        case ARM_INST_MOVT:
                                entry->value.i |= instr.value[1] << 16;
                                break;

                        case ARM_INST_BX:
                        case ARM_INST_BLX:
                                entry->type = ENTRY_TYPES_FUNCTION;
                                return ANALYZE_STUB_OK;

                        case ARM_INST_SVC:
                                entry->type = ENTRY_TYPES_SYSCALL;
                                return ANALYZE_STUB_OK;

                        case ARM_INST_MVN:
                                return ANALYZE_STUB_UNRESOLVED;

                        default:
                                DEBUG_LOG_("ERROR");
                                return ANALYZE_STUB_INVAL;
                }
                stub = (char*)stub + sizeof(SceUInt);
        }
}

int nid_table_isValidModuleInfo(SceModuleInfo *m_info)
{
        if(m_info == NULL) return 0; //Invalid if NULL
        if(m_info->modattribute != SCE_MODULE_INFO_EXPECTED_ATTR) return 0;
        if(m_info->modversion != SCE_MODULE_INFO_EXPECTED_VER) return 0;
        return 1;
}

__attribute__((hot))
int nid_table_addStubsInModule(Psp2LoadedModuleInfo *target)
{
        nidTable_entry entry;
        DEBUG_LOG_("Searching for module info");
        SceModuleInfo *orig_mod_info = nid_table_findModuleInfo(target->segments[0].vaddr, target->segments[0].memsz, target->module_name);
        DEBUG_LOG_("Found");
        if(orig_mod_info != NULL) {

                //Build entries from export table
                SceUInt base_orig = (SceUInt)orig_mod_info - orig_mod_info->ent_top + sizeof(SceModuleInfo);
                SceModuleExports *exportTable_orig = (SceModuleExports*)(base_orig + orig_mod_info->ent_top);

                for(; (SceUInt)exportTable_orig < (SceUInt)(base_orig + orig_mod_info->ent_end); exportTable_orig++)
                {
                        for(int i = 0; i < exportTable_orig->num_functions; i++)
                        {
                                entry.nid = exportTable_orig->nid_table[i];
                                entry.type = ENTRY_TYPES_FUNCTION;
                                entry.value.p = exportTable_orig->entry_table[i];
                                nid_storage_addEntry(&entry);
                        }
                }
                DEBUG_LOG_("Exports resolved");
                //NOTE: The problem is somewhere here

                //Build entries from import table

                Psp2LoadedModuleInfo l_mod_info;

                int loadResult = sizeof(int);
                SceUID l_mod_uid = sceKernelLoadModule(target->path,0,&loadResult);
                if(l_mod_uid < 0) {
                        DEBUG_LOG_("Reload failed...");
                        return -1;
                }

                l_mod_info.size = sizeof(Psp2LoadedModuleInfo);
                if(sceKernelGetModuleInfo(l_mod_uid, &l_mod_info) < 0) {
                        DEBUG_LOG_("Failed to get module info...");
                        return -1;
                }


                SceModuleInfo *mod_info = nid_table_findModuleInfo(l_mod_info.segments[0].vaddr, l_mod_info.segments[0].memsz, l_mod_info.module_name);

                if(mod_info != NULL)
                {
                        SceUInt base_orig = (SceUInt)target->segments[0].vaddr;

                        SceUInt base_l = (SceUInt)l_mod_info.segments[0].vaddr;
                        SceModuleImports *importTable_l = (SceModuleImports*)(base_l + mod_info->stub_top);

                        //Check the export tables to see if the NID is present
                        //If so assign the respective pointer to functionPtrLocation
                        FOREACH_IMPORT(base_orig, orig_mod_info, importTable_orig)
                        {
                                void **entryTable = GET_FUNCTIONS_ENTRYTABLE(importTable_orig);
                                SceNID *nidTable = GET_FUNCTIONS_NIDTABLE(importTable_l);

                                for(unsigned int i = 0; i < GET_FUNCTION_COUNT(importTable_orig); i++)
                                {
                                        int err = nid_table_analyzeStub(entryTable[i], nidTable[i], &entry);
                                        if(err == ANALYZE_STUB_OK)
                                               nid_storage_addEntry(&entry);
                                        else if(err == ANALYZE_STUB_INVAL)
                                               break;
                                }

                                entryTable = GET_VARIABLE_ENTRYTABLE(importTable_orig);
                                nidTable = GET_VARIABLE_NIDTABLE(importTable_l);

                                for(int i = 0; i < GET_VARIABLE_COUNT(importTable_orig); i++)
                                {
                                        entry.type = ENTRY_TYPES_VARIABLE;
                                        entry.nid = nidTable[i];
                                        entry.value.i = *(SceUInt*)entryTable[i];
                                        nid_storage_addEntry(&entry);
                                }

                                importTable_l = GET_NEXT_IMPORT(importTable_l);
                        }
                        DEBUG_LOG_("NID cache updated");
                }

                sceKernelUnloadModule(l_mod_uid);
        }
        return 0;
}

void nid_table_addAllHooks()
{
        nidTable_entry entry;
        uintptr_t top;
        unsigned int i;

        __asm__ ("addAllHooksPc: mov %0, pc;"
                 "ldr %1, =addAllHooksPc + 4;"
                 "sub %0, %0, %1;"
                 : "=r"(top), "=r"(i));
        for (i = 0; i < sizeof(forcedHooks) / sizeof(hook_t); i++) {
                entry.nid = forcedHooks[i].nid;
                entry.type = ENTRY_TYPES_FUNCTION;
                entry.value.i = top + (uintptr_t)forcedHooks[i].p;
                nid_storage_addEntry(&entry);
        }
}

int nid_table_addAllStubs()
{
        SceUID uids[NID_TABLE_MAX_MODULES];
        unsigned int numEntries = NID_TABLE_MAX_MODULES;

        int err = sceKernelGetModuleList(0xFF, uids, &numEntries);
        if(err < 0) {
                DEBUG_LOG_("Failed to get module list... Exiting...");
                return -1;
        }
        Psp2LoadedModuleInfo loadedModuleInfo;
        loadedModuleInfo.size = sizeof(loadedModuleInfo);

        for(unsigned int i = 0; i < numEntries; i++)
        {
                if(sceKernelGetModuleInfo(uids[i], &loadedModuleInfo) < 0) {
                        DEBUG_LOG_("Failed to get module info... Skipping...");
                }else{
                        DEBUG_LOG_("Mod info obtained");
                        nid_table_addStubsInModule(&loadedModuleInfo);
                }
        }
        DEBUG_LOG_("All modules resolved");
        return 0;
}

SceModuleInfo* nid_table_findModuleInfo(void* location, SceUInt size, char* libname)
{
        SceModuleInfo *moduleInfo = NULL;
        void *origLocation = location;
        //Find the module info string in this memory region
        while(size > 0)
        {
                DEBUG_LOG_("test");
                location = memstr(location, size, libname, strlen(libname));
                if(location == NULL)
                {
                        DEBUG_LOG_("Failed to find module info");
                        break;
                }

                //Perform a detailed check to make sure this is a real match
                moduleInfo = (SceModuleInfo*)((SceUInt)location - 4);
                if(nid_table_isValidModuleInfo(moduleInfo)) break;
                else {
                        DEBUG_LOG_("False alarm...Continuing...");
                        moduleInfo = NULL; //If check fails, this was a false positive and move on
                        size -= ((SceUInt)location - (SceUInt)origLocation) + strlen(libname);
                        location = (void*)(((SceUInt)location) + strlen(libname));
                }
        }

        return moduleInfo;
}

__attribute__((hot))
int nid_table_addNIDCacheToTable(const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM])
{
        nidTable_entry entry;
        NID_CACHE *importsInfo;
        SceNID *cachedNid;
        unsigned int index, offset, i;

        offset = 0;
        importsInfo = nidCache_getHeader();
        cachedNid = nidCache_getCache();
        for (index = 0; index < CACHED_IMPORTED_MODULE_NUM; index++) {
                for(i = 0; i < importsInfo[index].count; i++) {
                        if(nid_table_analyzeStub(GET_FUNCTIONS_ENTRYTABLE(cachedImports[index])[i], cachedNid[offset], &entry) == ANALYZE_STUB_OK)
                                nid_storage_addEntry(&entry);
                        offset++;
                }
        }
        return 0;
}

static void copyStub(void *dst, const void *src)
{
        memcpy(dst, src, 16);
}

static int resolveVhlImportWithTable(SceUInt *stub, const UVL_Context *ctx)
{
        nidTable_entry entry;
        int res;

        //Check the cache if it's ready
        res = nid_storage_getEntry(stub[3], &entry);
        if (res == 0) {
                ctx->psvUnlockMem();
                resolveStubWithEntry(stub, &entry);
                ctx->psvLockMem();
        }

        return res;
}

static int resolveVhlImportWithCache(SceUInt *stub,
        const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM],
        const UVL_Context *ctx)
{
        unsigned int index, offset, i;
        NID_CACHE *importsInfo;
        SceNID *cachedNid;

        offset = 0;
        importsInfo = nidCache_getHeader();
        cachedNid = nidCache_getCache();
        for (index = 0; index < CACHED_IMPORTED_MODULE_NUM; index++) {
                for(i = 0; i < importsInfo[index].count; i++) {
                        if (cachedNid[offset] == stub[3]) {
                                ctx->psvUnlockMem();
                                copyStub(stub, GET_FUNCTIONS_ENTRYTABLE(cachedImports[index])[i]);
                                ctx->psvLockMem();

                                return 0;
                        }

                        offset++;
                }
        }

        return -1;
}

static int resolveVhlImportWithLibkernel(SceUInt *stub, const SceModuleInfo *moduleInfo,
                                         const UVL_Context *ctx)
{
        unsigned int i;
        SceNID nid;

        nid = stub[3];

        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);

        //Check the export tables to see if the NID is present
        //If so assign the respective pointer to functionPtrLocation
        FOREACH_EXPORT(base, moduleInfo, exportTable) {
                for(i = 0; i < exportTable->num_functions; i++)
                {
                        if(exportTable->nid_table[i] == nid) {
                                //Found the nid
                                ctx->psvUnlockMem();
                                resolveStubWithBranch(stub, exportTable->entry_table[i]);
                                ctx->psvLockMem();
                                return 0;
                        }
                }
        }


        FOREACH_IMPORT(base, moduleInfo, importTable)
        {
                SceUInt *nids = GET_FUNCTIONS_NIDTABLE(importTable);
                void **entryTable = GET_FUNCTIONS_ENTRYTABLE(importTable);

                for(i = 0; i < GET_FUNCTION_COUNT(importTable); i++)
                {
                        if(nids[i] == nid) {
                                ctx->psvUnlockMem();
                                copyStub(stub, entryTable[i]);
                                ctx->psvLockMem();

                                return 0;
                        }
                }
        }

        return -1;
}

void nid_table_resolveVhlPuts(void *p, const UVL_Context *ctx)
{
        ctx->psvUnlockMem();
        resolveStubWithBranch(p, ctx->logline);
        ctx->psvLockMem();
}

void nid_table_resolveVhlPrimaryImports(void *p, size_t size, const SceModuleInfo *libkernel,
        const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM],
        const UVL_Context *ctx)
{
        uintptr_t cur;
        uintptr_t btm = (uintptr_t)p + size;

        for (cur = (uintptr_t)p; cur < btm; cur += 16) {
                DEBUG_LOG_("Searching cache");
                if (!resolveVhlImportWithCache((void *)cur, cachedImports, ctx))
                        continue;

                DEBUG_LOG_("Searching sceLibKernel");
                if (!resolveVhlImportWithLibkernel((void *)cur, libkernel, ctx))
                        continue;

                DEBUG_LOG("Failed to find import NID 0x%08x", ((SceNID *)cur)[3]);
        }
}

void nid_table_resolveVhlSecondaryImports(void *p, size_t size, const SceModuleInfo *libkernel,
        const SceModuleImports * const cachedImports[CACHED_IMPORTED_MODULE_NUM],
        const UVL_Context *ctx)
{
        uintptr_t cur;
        uintptr_t btm = (uintptr_t)p + size;

        for (cur = (uintptr_t)p; cur < btm; cur += 16) {
                DEBUG_LOG_("Searching cache");
                if (!resolveVhlImportWithCache((void *)cur, cachedImports, ctx))
                        continue;

                DEBUG_LOG_("Searching sceLibKernel");
                if (!resolveVhlImportWithLibkernel((void *)cur, libkernel, ctx))
                        continue;

                DEBUG_LOG_("Searching NID database");
                if (!resolveVhlImportWithTable((void *)cur, ctx))
                        continue;

                DEBUG_LOG("Failed to find import NID 0x%08x", ((SceNID *)cur)[3]);
        }
}

__attribute__((hot))
int nid_table_resolveStub(void *stub, SceNID nid)
{
        nidTable_entry entry;
        int result;

        result = nid_storage_getEntry(nid, &entry);
        if(result >= 0) {
                sceKernelOpenVMDomain();
                resolveStubWithEntry((void*)((SceUInt)stub & ~1), &entry);
                sceKernelCloseVMDomain();

                return 0;
        }
        DEBUG_LOG("Failed to find NID 0x%08x", nid);
        return -1;
}
