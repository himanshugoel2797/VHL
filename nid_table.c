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
#include <psp2/pss.h>
#include <stdio.h>
#include "nid_table.h"
#include "stub.h"

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
                        pss_code_mem_unlock();
                        resolveStubWithBranch(stub, entry->value.p);
                        pss_code_mem_lock();
                        break;

                case ENTRY_TYPES_SYSCALL:
                        pss_code_mem_unlock();
                        resolveStubWithSvc(stub, entry->value.i);
                        pss_code_mem_lock();
                        break;

                case ENTRY_TYPES_VARIABLE:
                        pss_code_mem_unlock();
                        *(SceUInt*)stub = entry->value.i;
                        pss_code_mem_lock();
                        break;

                default:
                        return -1;
       }

        return 0;
}

enum {
        ANALYZE_STUB_OK,
        ANALYZE_STUB_UNRESOLVED,
        ANALYZE_STUB_INVAL
};

__attribute__((hot))
static int analyzeStub(const void *stub, SceNID nid, nidTable_entry *entry)
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

                                for(int i = 0; i < GET_FUNCTION_COUNT(importTable_orig); i++)
                                {
                                        int err = analyzeStub(entryTable[i], nidTable[i], &entry);
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

int nid_table_addAllStubs()
{
        SceUID uids[NID_TABLE_MAX_MODULES];
        int numEntries = NID_TABLE_MAX_MODULES;

        int err = sceKernelGetModuleList(0xFF, uids, &numEntries);
        if(err < 0) {
                DEBUG_LOG_("Failed to get module list... Exiting...");
                return -1;
        }
        Psp2LoadedModuleInfo loadedModuleInfo;
        loadedModuleInfo.size = sizeof(loadedModuleInfo);

        for(int i = 0; i < numEntries; i++)
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
int nid_table_addNIDCacheToTable(SceModuleInfo *moduleInfo)
{
        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);
        nidTable_entry entry;

        FOREACH_IMPORT(base, moduleInfo, importTable){
                int offset = 0;
                int index = nidCacheContainsModuleNID(GET_NID(importTable), &offset);
                if(index >= 0) {
                        NID_CACHE *libkernel_nid_cache_header = nidCache_getHeader();
                        SceNID *libkernel_nid_cache = nidCache_getCache();

                        for(int i = 0; i < libkernel_nid_cache_header[index].count; i++)
                        {
                                if(analyzeStub(GET_FUNCTIONS_ENTRYTABLE(importTable)[i], libkernel_nid_cache[offset + i], &entry) == ANALYZE_STUB_OK)
                                        nid_storage_addEntry(&entry);
                        }
                }
        }
        return 0;
}


static int resolveVhlImport(SceUInt *stub, const SceModuleInfo *moduleInfo)
{
        if (stub == NULL || moduleInfo == NULL)
                return -1;

        //Check the cache if it's ready
        DEBUG_LOG_("Searching NID database...");
        nidTable_entry entry;
        if(nid_storage_getEntry(stub[3], &entry) >= 0) {
                resolveStubWithEntry(stub, &entry);
                return 0;
        }

        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);

        //Check the export tables to see if the NID is present
        //If so assign the respective pointer to functionPtrLocation
        FOREACH_EXPORT(base, moduleInfo, exportTable) {
                DEBUG_LOG_("Searching exports...");
                for(int i = 0; i < exportTable->num_functions; i++)
                {
                        if(exportTable->nid_table[i] == stub[3]) {
                                //Found the nid
                                DEBUG_LOG_("Match found!");
                                pss_code_mem_unlock();
                                resolveStubWithBranch(stub, exportTable->entry_table[i]);
                                pss_code_mem_lock();
                                return 0;
                        }
                }
        }


        FOREACH_IMPORT(base, moduleInfo, importTable)
        {
                DEBUG_LOG_("Searching imports...");

                SceUInt *nids = GET_FUNCTIONS_NIDTABLE(importTable);
                void **entryTable = GET_FUNCTIONS_ENTRYTABLE(importTable);

                for(int i = 0; i < GET_FUNCTION_COUNT(importTable); i++)
                {
                        if(nids[i] == stub[3]) {
                                DEBUG_LOG_("Match found!");
                                pss_code_mem_unlock();
                                memcpy(stub, entryTable[i], 16);
                                pss_code_mem_lock();
                                return 0;
                        }
                }
        }
        DEBUG_LOG("Failed to find import NID 0x%08x", stub[3]);

        return -1;
}

int nid_table_resolveVHLImports(const UVL_Context *ctx)
{
        SceUInt * const vhlStubTop = getVhlStubTop();
        SceUInt * const vhlStubCtxBtm = (void *)((uintptr_t)vhlStubTop + vhlStubCtxSize);
        SceUInt * const vhlStubPrimaryBtm = (void *)((uintptr_t)vhlStubCtxBtm + vhlStubPrimarySize);
        SceUInt * const vhlStubSecondaryBtm = (void *)((uintptr_t)vhlStubPrimaryBtm + vhlStubSecondarySize);
        nidTable_entry libKernelBase, entry;
        SceUInt *p;
        const void * const *q;

        ctx->funcs.logline("Resolving VHL Imports");

        ctx->funcs.psvUnlockMem();

        q = ctx->ptrs.funcs;
        for (p = vhlStubTop; p != vhlStubCtxBtm - 4; p += 4) {
                memcpy(p, *q, 16);
                q++;
        }

        // puts
        resolveStubWithBranch(p, *q);

        ctx->funcs.psvLockMem();
        ctx->funcs.psvFlushIcache(vhlStubTop, vhlStubCtxSize);
        // pss_* and puts are avaliable now.

        if(analyzeStub(ctx->ptrs.libkernel_anchor, 0, &libKernelBase) == ANALYZE_STUB_OK) {

                DEBUG_LOG_("Initializing cache");
                nid_storage_initialize();

                libKernelBase.value.i = B_UNSET(libKernelBase.value.i, 0);

                SceModuleInfo *moduleInfo = nid_table_findModuleInfo(libKernelBase.value.p, KERNEL_MODULE_SIZE, "SceLibKernel");
                if (moduleInfo == NULL) return -1;

                SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);

                if(nid_table_addNIDCacheToTable(moduleInfo) < 0) return -1;

                for (p = vhlStubCtxBtm; p != vhlStubPrimaryBtm; p += 4)
                        resolveVhlImport(p, moduleInfo);

                pss_code_mem_flush_icache(vhlStubCtxBtm, vhlStubPrimarySize);

                DEBUG_LOG_("Searching and Caching NIDs...");
                nid_table_addAllStubs();

                while (p != vhlStubSecondaryBtm) {
                        resolveVhlImport(p, moduleInfo);
                        p += 4;
                }

                pss_code_mem_flush_icache(vhlStubPrimaryBtm, vhlStubSecondarySize);
        }

        return 0;
}

int nid_table_exportFunc(void *target, SceNID nid)
{
        nidTable_entry entry;
        entry.nid = nid;
        entry.type = ENTRY_TYPES_FUNCTION;
        entry.value.p = target;

        nid_storage_addEntry(&entry);

        return 0;
}

__attribute__((hot))
int nid_table_resolveStub(int priority, void *stub, SceNID nid)
{
        nidTable_entry entry;

        int result = 0;
        if(priority > 0) result = nid_storage_getHookEntry(nid, &entry);
        if(priority <= 0 || result < 0) result = nid_storage_getEntry(nid, &entry);

        if(result >= 0) {
                stub = (void*)((SceUInt)stub & ~1);

                resolveStubWithEntry(stub, &entry);
                if (entry.type != ENTRY_TYPES_VARIABLE)
                        pss_code_mem_flush_icache(stub, 0x10);

                return 0;
        }
        DEBUG_LOG("Failed to find NID 0x%08x", nid);
        return -1;
}

int nid_table_registerHook(void *func, SceNID nid)
{
        nidTable_entry entry;
        if(nid_storage_getEntry(nid, &entry) >= 0)
        {
                entry.nid = nid;
                entry.type = ENTRY_TYPES_FUNCTION;
                entry.value.p = func;
                nid_storage_addHookEntry(&entry);
                return 0;
        }
        return -1;
}
