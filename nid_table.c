#include "nid_table.h"


int resolveStub(void *stub, SceNID nid, nidTable_entry *entry)
{
        entry->nid = nid;
        entry->type = ENTRY_TYPES_UNKN;
        entry->value.location = 0;
        entry->stub_loc = stub;

        ARM_INSTRUCTION instr;

        while(entry->type == ENTRY_TYPES_UNKN)
        {
                if(Disassemble(stub, &instr) >= 0)
                {
                        switch(instr.instruction)
                        {
                        case ARM_INST_MOVW:
                                entry->value.location = instr.value[1];
                                break;
                        case ARM_INST_MOVT:
                                entry->value.location |= instr.value[1] << 16;
                                break;

                        case ARM_INST_BX:
                        case ARM_INST_BLX:
                                entry->type = ENTRY_TYPES_FUNCTION;
                                break;

                        case ARM_INST_SVC:
                                entry->type = ENTRY_TYPES_SYSCALL;
                                break;

                        case ARM_INST_ADR:
                                entry->type = ENTRY_TYPES_RELOC;
                                break;

                        case ARM_INST_MVN:
                                entry->nid = 0;
                                return -1;
                                break;

                        default:
                                DEBUG_LOG_("ERROR");
                                entry->nid = 0;
                                return -1;
                                break;
                        }
                }
                stub = (char*)stub + sizeof(SceUInt);
        }
        return 0;
}

int nidTable_isValidModuleInfo(SceModuleInfo *m_info)
{
        if(m_info == NULL) return 0; //Invalid if NULL
        if(m_info->modattribute != SCE_MODULE_INFO_EXPECTED_ATTR) return 0;
        if(m_info->modversion != SCE_MODULE_INFO_EXPECTED_VER) return 0;
        return 1;
}

int nidTable_resolveFromModule(VHLCalls *calls, SceLoadedModuleInfo *target)
{
        nidTable_entry entry;
        SceModuleInfo *orig_mod_info = nidTable_findModuleInfo(target->segments[0].vaddr, target->segments[0].memsz, target->module_name);
        if(orig_mod_info != NULL) {

                //Build entries from export table
                SceUInt base_orig = (SceUInt)orig_mod_info - orig_mod_info->ent_top + sizeof(SceModuleInfo);
                SceModuleExports *exportTable_orig = (SceModuleExports*)(base_orig + orig_mod_info->ent_top);

                calls->UnlockMem();
                for(; (SceUInt)exportTable_orig < (SceUInt)(base_orig + orig_mod_info->ent_end); exportTable_orig++)
                {
                        for(int i = 0; i < exportTable_orig->num_functions; i++)
                        {
                                int err = resolveStub(exportTable_orig->entry_table[i], exportTable_orig->nid_table[i], &entry);
                                if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                        }
                }
                calls->LockMem();

                //Build entries from import table

                SceLoadedModuleInfo l_mod_info;

                int loadResult = sizeof(int);
                SceUID l_mod_uid = calls->sceKernelLoadModule(target->file_path,0,&loadResult);
                if(l_mod_uid < 0) {
                        DEBUG_LOG_("Reload failed...");
                        return -1;
                }

                l_mod_info.size = sizeof(SceLoadedModuleInfo);
                if(calls->sceKernelGetModuleInfo(l_mod_uid, &l_mod_info) < 0) {
                        DEBUG_LOG_("Failed to get module info...");
                        return -1;
                }


                SceModuleInfo *mod_info = nidTable_findModuleInfo(l_mod_info.segments[0].vaddr, l_mod_info.segments[0].memsz, l_mod_info.module_name);

                if(mod_info != NULL)
                {
                        SceUInt base_orig = (SceUInt)target->segments[0].vaddr;
                        SceModuleImports *importTable_orig = (SceModuleImports*)(base_orig + orig_mod_info->stub_top);

                        SceUInt base_l = (SceUInt)l_mod_info.segments[0].vaddr;
                        SceModuleImports *importTable_l = (SceModuleImports*)(base_l + mod_info->stub_top);

                        //Check the export tables to see if the NID is present
                        //If so assign the respective pointer to functionPtrLocation
                        calls->UnlockMem();
                        for(; (SceUInt)importTable_orig < (SceUInt)(base_orig + orig_mod_info->stub_end); importTable_orig= (SceModuleImports*)((SceUInt)importTable_orig + importTable_orig->size))
                        {
                                for(int i = 0; i < SCE_MODULE_IMPORTS_GET_FUNCTION_COUNT(importTable_orig); i++)
                                {
                                        int err = resolveStub(SCE_MODULE_IMPORTS_GET_FUNCTIONS_ENTRYTABLE(importTable_orig)[i], SCE_MODULE_IMPORTS_GET_FUNCTIONS_NIDTABLE(importTable_l)[i], &entry);
                                        if(entry.nid == 2968642796) DEBUG_LOG("MATCH FOUND! 0x%08x 0x%08x", entry.type, entry.value.location);
                                        if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                                }

                                for(int i = 0; i < SCE_MODULE_IMPORTS_GET_VARIABLE_COUNT(importTable_orig); i++)
                                {
                                        entry.type = ENTRY_TYPES_VARIABLE;
                                        entry.nid = SCE_MODULE_IMPORTS_GET_VARIABLE_NIDTABLE(importTable_l)[i];
                                        entry.stub_loc = SCE_MODULE_IMPORTS_GET_VARIABLE_ENTRYTABLE(importTable_orig)[i];
                                        entry.value.location = *(SceUInt*)entry.stub_loc;
                                        if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                                }

                                importTable_l = (SceModuleImports*)((SceUInt)importTable_l + importTable_l->size);
                        }
                        calls->LockMem();
                        DEBUG_LOG_("NID cache updated");
                }

                calls->sceKernelUnloadModule(l_mod_uid);
        }
        return 0;
}

int nidTable_resolveAll(VHLCalls *calls)
{
        SceUID uids[NID_TABLE_MAX_MODULES];
        SceInt numEntries = NID_TABLE_MAX_MODULES;

        int err = calls->sceKernelGetModuleList(0xFF, uids, &numEntries);
        if(err < 0) {
                DEBUG_LOG_("Failed to get module list... Exiting...");
                return -1;
        }
        SceLoadedModuleInfo loadedModuleInfo;

        for(int i = 0; i < numEntries; i++)
        {
                loadedModuleInfo.size = sizeof(loadedModuleInfo);
                if(calls->sceKernelGetModuleInfo(uids[i], &loadedModuleInfo) < 0) {
                        DEBUG_LOG_("Failed to get module info... Skipping...");
                }else{
                        nidTable_resolveFromModule(calls, &loadedModuleInfo);
                }
        }
        return 0;
}

SceModuleInfo* nidTable_findModuleInfo(void* location, SceUInt size, char* libname)
{
        SceModuleInfo *moduleInfo = NULL;
        void *origLocation = location;
        //Find the module info string in this memory region
        while(size > 0)
        {
                location = memstr(location, size, libname, strlen(libname));
                if(location == NULL)
                {
                        DEBUG_LOG_("Failed to find module info");
                        break;
                }

                //Perform a detailed check to make sure this is a real match
                moduleInfo = (SceModuleInfo*)((SceUInt)location - 4);
                if(nidTable_isValidModuleInfo(moduleInfo)) break;
                else {
                        DEBUG_LOG_("False alarm...Continuing...");
                        moduleInfo = NULL; //If check fails, this was a false positive and move on
                        size -= ((SceUInt)location - (SceUInt)origLocation) + strlen(libname);
                        location = ((SceUInt)location) + strlen(libname);
                }
        }

        return moduleInfo;
}

int nidTable_addNIDCacheToTable(VHLCalls *calls, void *libraryBase)
{
        SceModuleInfo *moduleInfo = nidTable_findModuleInfo(libraryBase, KERNEL_MODULE_SIZE, "SceLibKernel");
        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);
        SceModuleImports *importTable = (SceModuleImports*)(base + moduleInfo->stub_top);
        nidTable_entry entry;

        for(; (SceUInt)importTable < (SceUInt)(base + moduleInfo->stub_end); importTable=(SceModuleImports*)((SceUInt)importTable + importTable->size)) {
                int offset = 0;
                int index = nidCacheContainsModuleNID(SCE_MODULE_IMPORTS_GET_NID(importTable), &offset);
                if(index >= 0) {
                        NID_CACHE *libkernel_nid_cache_header = nidCache_getHeader();
                        SceNID *libkernel_nid_cache = nidCache_getCache();

                        for(int i = 0; i < libkernel_nid_cache_header[index].count; i++) {
                                resolveStub(SCE_MODULE_IMPORTS_GET_FUNCTIONS_ENTRYTABLE(importTable)[i], libkernel_nid_cache[offset + i], &entry);
                                if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                        }
                }
        }
}


int nidTable_resolveImportFromNID(VHLCalls *calls, SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName)
{
        //Find the module info struct
        SceModuleInfo *moduleInfo = nidTable_findModuleInfo(libraryBase, KERNEL_MODULE_SIZE, libName);
        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);
        SceModuleExports *exportTable = (SceModuleExports*)(base + moduleInfo->ent_top);
        SceModuleImports *importTable = (SceModuleImports*)(base + moduleInfo->stub_top);

        DEBUG_LOG("Searching for NID 0x%08x in module %s", nid, libName);

        calls->UnlockMem();
        //Check the cache if it's ready
        DEBUG_LOG_("Searching NID database...");
        nidTable_entry entry;
        if(nid_storage_getEntry(nid, &entry) >= 0) {
                *functionPtrLocation = entry.stub_loc;
                calls->LockMem();
                return 0;
        }

        //Check the export tables to see if the NID is present
        //If so assign the respective pointer to functionPtrLocation
        for(; (SceUInt)exportTable < (SceUInt)(base + moduleInfo->ent_end); exportTable++) {
                DEBUG_LOG_("Searching exports...");
                for(int i = 0; i < exportTable->num_functions; i++)
                {
                        if(exportTable->nid_table[i] == nid) {
                                //Found the nid
                                DEBUG_LOG_("Match found!");
                                *functionPtrLocation = exportTable->entry_table[i];
                                calls->LockMem();
                                return 0;
                        }
                }
        }


        for(; (SceUInt)importTable < (SceUInt)(base + moduleInfo->stub_end); importTable=(SceModuleImports*)((SceUInt)importTable + importTable->size)) {
                DEBUG_LOG_("Searching imports...");
                for(int i = 0; i < SCE_MODULE_IMPORTS_GET_FUNCTION_COUNT(importTable); i++)
                {
                        if(SCE_MODULE_IMPORTS_GET_FUNCTIONS_NIDTABLE(importTable)[i] == nid) {
                                DEBUG_LOG_("Match found!");
                                *functionPtrLocation = SCE_MODULE_IMPORTS_GET_FUNCTIONS_ENTRYTABLE(importTable)[i];
                                calls->LockMem();
                                return 0;
                        }
                }
        }

        calls->LockMem();
        DEBUG_LOG("Failed to find import NID 0x%08x in imports for %s", nid, libName);

        return -1;
}

#define STUB_FUNCTION(type, name) \
        type __attribute__((naked, section(".sceStub.text.uvl"))) name () \
        { \
                __asm__ ("movw r0, #0xffff\n" \
                         "movt r0, #0xffff\n" \
                         "bx lr\n"); \
        }

STUB_FUNCTION(SceUID, ioOpen);

int nidTable_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls)
{
        nidTable_entry libKernelBase;
        DEBUG_LOG_("Resolving VHL Imports");
        if(resolveStub(ctx->libkernel_anchor, 0, &libKernelBase) >= 0) {

                nid_storage_initialize(calls);
                DEBUG_LOG_("Initializing cache");

                nidTable_addNIDCacheToTable(calls, libKernelBase.value.location);

                libKernelBase.value.location = B_UNSET(libKernelBase.value.location, 0);
                int err = nidTable_resolveImportFromNID(calls,(SceUInt*)&calls->sceKernelGetModuleList,
                                                        SCE_KERNEL_GET_MODULE_LIST,
                                                        libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetModuleInfo,
                                                    SCE_KERNEL_GET_MODULE_INFO,
                                                    libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelLoadModule,
                                                    SCE_KERNEL_LOADMODULE,
                                                    libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelUnloadModule,
                                                    SCE_KERNEL_UNLOADMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                DEBUG_LOG_("Resolving and Caching NIDs...");
                nidTable_resolveAll(calls);

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOOpen,
                                                    SCE_IO_OPEN,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelLoadStartModule,
                                                    SCE_KERNEL_LOAD_STARTMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelStopUnloadModule,
                                                    SCE_KERNEL_STOP_UNLOADMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOMkdir,
                                                    SCE_IO_MKDIR,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelAllocMemBlock,
                                                    SCE_KERNEL_ALLOC_MEMBLOCK,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetMemBlockBase,
                                                    SCE_KERNEL_GET_MEMBLOCK_BASE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOClose,
                                                    SCE_IO_CLOSE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIORead,
                                                    SCE_IO_READ,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOWrite,
                                                    SCE_IO_WRITE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOLseek,
                                                    SCE_IO_LSEEK,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelFindMemBlockByAddr,
                                                    SCE_KERNEL_FIND_MEMBLOCK_BY_ADDR,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                return 0;
        }
        return -1;
}

int nidTable_exportFunc(VHLCalls *calls, void *target, SceNID nid)
{
        calls->UnlockMem();

        nidTable_entry entry;
        entry.nid = nid;
        entry.type = ENTRY_TYPES_FUNCTION;
        entry.value.location = (SceUInt)target;
        entry.stub_loc = (SceUInt)target;

        nid_storage_addEntry(calls, &entry);

        calls->LockMem();
        return 0;
}

int nidTable_setNIDaddress(VHLCalls *calls, void *stub, SceNID nid)
{
        nidTable_entry entry;
        if(nid_storage_getEntry(nid, &entry) >= 0) {
                if(entry.nid == nid) {
                        //Fill the stub with the code generated for this entry
                        ARM_INSTRUCTION movt;
                        ARM_INSTRUCTION movw;
                        ARM_INSTRUCTION jmp;

                        stub = (SceUInt)stub & ~1;

                        switch(entry.type) {
                        /*case ENTRY_TYPES_FUNCTION:

                                movw.condition = ARM_CONDITION_ALWAYS;
                                movw.type = ARM_MOV_INSTRUCTION;
                                movw.instruction = ARM_INST_MOVW;
                                movw.argCount = 2;
                                movw.value[0] = ARM_R12;
                                movw.value[1] = (SceUInt16)entry.value.location;

                                movt.condition = ARM_CONDITION_ALWAYS;
                                movt.type = ARM_MOV_INSTRUCTION;
                                movt.instruction = ARM_INST_MOVT;
                                movt.argCount = 2;
                                movt.value[0] = ARM_R12;
                                movt.value[1] = (SceUInt16)(entry.value.location >> 16); //Only the top part

                                jmp.condition = ARM_CONDITION_ALWAYS;
                                jmp.type = ARM_BRANCH_INSTRUCTION;
                                jmp.instruction = ARM_INST_BX;
                                jmp.argCount = 1;
                                jmp.value[0] = ARM_R12;
                                break;
                           case ENTRY_TYPES_SYSCALL:
                                movw.condition = ARM_CONDITION_ALWAYS;
                                movw.type = ARM_MOV_INSTRUCTION;
                                movw.instruction = ARM_INST_MOVW;
                                movw.argCount = 2;
                                movw.value[0] = ARM_R12;
                                movw.value[1] = entry.value.location;

                                movt.condition = ARM_CONDITION_ALWAYS;
                                movt.type = ARM_SVC_INSTRUCTION;
                                movt.instruction = ARM_INST_SVC;
                                movt.argCount = 1;
                                movt.value[0] = 0;

                                jmp.condition = ARM_CONDITION_ALWAYS;
                                jmp.type = ARM_BRANCH_INSTRUCTION;
                                jmp.instruction = ARM_INST_BX;
                                jmp.argCount = 1;
                                jmp.value[0] = ARM_R14;
                                break;*/
                        case ENTRY_TYPES_SYSCALL:
                        case ENTRY_TYPES_RELOC:
                        case ENTRY_TYPES_FUNCTION:

                                movw.condition = ARM_CONDITION_ALWAYS;
                                movw.type = ARM_MOV_INSTRUCTION;
                                movw.instruction = ARM_INST_MOVW;
                                movw.argCount = 2;
                                movw.value[0] = ARM_R12;
                                movw.value[1] = (SceUInt16)entry.stub_loc;

                                movt.condition = ARM_CONDITION_ALWAYS;
                                movt.type = ARM_MOV_INSTRUCTION;
                                movt.instruction = ARM_INST_MOVT;
                                movt.argCount = 2;
                                movt.value[0] = ARM_R12;
                                movt.value[1] = (SceUInt16)((SceUInt)entry.stub_loc >> 16);                  //Only the top part

                                jmp.condition = ARM_CONDITION_ALWAYS;
                                jmp.type = ARM_BRANCH_INSTRUCTION;
                                jmp.instruction = ARM_INST_BX;
                                jmp.argCount = 1;
                                jmp.value[0] = ARM_R12;
                                break;
                        case ENTRY_TYPES_VARIABLE:
                                calls->UnlockMem();
                                *(SceUInt*)stub = entry.value.location;
                                calls->LockMem();
                                return 0;
                                break;
                        default:
                                return -1;
                                break;
                        }


                        calls->UnlockMem();
                        Assemble(&movw, &((SceUInt*)stub)[0]);
                        Assemble(&movt, &((SceUInt*)stub)[1]);
                        Assemble(&jmp, &((SceUInt*)stub)[2]);

                        calls->LockMem();
                        calls->FlushICache(stub, 0x10);
                        return 0;
                }
        }
        return -1;
}
