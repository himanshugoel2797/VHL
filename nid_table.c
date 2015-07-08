#include "nid_table.h"


int nid_table_analyzeStub(void *stub, SceNID nid, nidTable_entry *entry)
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

int nid_table_isValidModuleInfo(SceModuleInfo *m_info)
{
        if(m_info == NULL) return 0; //Invalid if NULL
        if(m_info->modattribute != SCE_MODULE_INFO_EXPECTED_ATTR) return 0;
        if(m_info->modversion != SCE_MODULE_INFO_EXPECTED_VER) return 0;
        return 1;
}

int nid_table_resolveFromModule(VHLCalls *calls, SceLoadedModuleInfo *target)
{
        nidTable_entry entry;
        DEBUG_LOG_("Searching for module info");
        SceModuleInfo *orig_mod_info = nid_table_findModuleInfo(target->segments[0].vaddr, target->segments[0].memsz, target->module_name);
        if(orig_mod_info != NULL) {

                //Build entries from export table
                SceUInt base_orig = (SceUInt)orig_mod_info - orig_mod_info->ent_top + sizeof(SceModuleInfo);
                SceModuleExports *exportTable_orig = (SceModuleExports*)(base_orig + orig_mod_info->ent_top);

                calls->UnlockMem();
                for(; (SceUInt)exportTable_orig < (SceUInt)(base_orig + orig_mod_info->ent_end); exportTable_orig++)
                {
                        for(int i = 0; i < exportTable_orig->num_functions; i++)
                        {
                                int err = nid_table_analyzeStub(exportTable_orig->entry_table[i], exportTable_orig->nid_table[i], &entry);
                                if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                        }
                }
                calls->LockMem();
                DEBUG_LOG_("Exports resolved");
                //NOTE: The problem is somewhere here

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


                SceModuleInfo *mod_info = nid_table_findModuleInfo(l_mod_info.segments[0].vaddr, l_mod_info.segments[0].memsz, l_mod_info.module_name);

                if(mod_info != NULL)
                {
                        SceUInt base_orig = (SceUInt)target->segments[0].vaddr;

                        SceUInt base_l = (SceUInt)l_mod_info.segments[0].vaddr;
                        SceModuleImports *importTable_l = (SceModuleImports*)(base_l + mod_info->stub_top);

                        //Check the export tables to see if the NID is present
                        //If so assign the respective pointer to functionPtrLocation
                        calls->UnlockMem();
                        FOREACH_IMPORT(base_orig, orig_mod_info, importTable_orig)
                        {
                                void **entryTable = GET_FUNCTIONS_ENTRYTABLE(importTable_orig);
                                SceNID *nidTable = GET_FUNCTIONS_NIDTABLE(importTable_l);

                                for(int i = 0; i < GET_FUNCTION_COUNT(importTable_orig); i++)
                                {
                                        int err = nid_table_analyzeStub(entryTable[i], nidTable[i], &entry);
                                        if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                                }

                                entryTable = GET_VARIABLE_ENTRYTABLE(importTable_orig);
                                nidTable = GET_VARIABLE_NIDTABLE(importTable_l);

                                for(int i = 0; i < GET_VARIABLE_COUNT(importTable_orig); i++)
                                {
                                        entry.type = ENTRY_TYPES_VARIABLE;
                                        entry.nid = nidTable[i];
                                        entry.stub_loc = entryTable[i];
                                        entry.value.location = *(SceUInt*)entry.stub_loc;
                                        if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                                }

                                importTable_l = GET_NEXT_IMPORT(importTable_l);
                        }
                        calls->LockMem();
                        DEBUG_LOG_("NID cache updated");
                }

                calls->sceKernelUnloadModule(l_mod_uid);
        }
        return 0;
}

int nid_table_resolveAll(VHLCalls *calls)
{
        SceUID uids[NID_TABLE_MAX_MODULES];
        int numEntries = NID_TABLE_MAX_MODULES;

        int err = calls->sceKernelGetModuleList(0xFF, uids, &numEntries);
        if(err < 0) {
                DEBUG_LOG_("Failed to get module list... Exiting...");
                return -1;
        }
        SceLoadedModuleInfo loadedModuleInfo;
        loadedModuleInfo.size = sizeof(loadedModuleInfo);

        for(int i = 0; i < numEntries; i++)
        {
                if(calls->sceKernelGetModuleInfo(uids[i], &loadedModuleInfo) < 0) {
                        DEBUG_LOG_("Failed to get module info... Skipping...");
                }else{
                        DEBUG_LOG_("Mod info obtained");
                        nid_table_resolveFromModule(calls, &loadedModuleInfo);
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

int nid_table_addNIDCacheToTable(VHLCalls *calls, void *libraryBase)
{
        SceModuleInfo *moduleInfo = nid_table_findModuleInfo(libraryBase, KERNEL_MODULE_SIZE, "SceLibKernel");
        if(moduleInfo == NULL) return -1;

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
                                nid_table_analyzeStub(GET_FUNCTIONS_ENTRYTABLE(importTable)[i], libkernel_nid_cache[offset + i], &entry);
                                if(entry.nid != 0) nid_storage_addEntry(calls, &entry);
                        }
                }
        }
        return 0;
}


int nid_table_resolveImportFromNID(VHLCalls *calls, SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName)
{

        if(libName != NULL)DEBUG_LOG("Searching for NID 0x%08x in module %s", nid, libName);

        calls->UnlockMem();
        //Check the cache if it's ready
        DEBUG_LOG_("Searching NID database...");
        nidTable_entry entry;
        if(nid_storage_getEntry(nid, &entry) >= 0) {
                *functionPtrLocation = (SceUInt)entry.stub_loc;
                calls->LockMem();
                return 0;
        }

        if(libraryBase == NULL | libName == NULL)return -1;

        //Find the module info struct
        SceModuleInfo *moduleInfo = nid_table_findModuleInfo(libraryBase, KERNEL_MODULE_SIZE, libName);
        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);

        //Check the export tables to see if the NID is present
        //If so assign the respective pointer to functionPtrLocation
        FOREACH_EXPORT(base, moduleInfo, exportTable) {
                DEBUG_LOG_("Searching exports...");
                for(int i = 0; i < exportTable->num_functions; i++)
                {
                        if(exportTable->nid_table[i] == nid) {
                                //Found the nid
                                DEBUG_LOG_("Match found!");
                                *functionPtrLocation = (SceUInt)exportTable->entry_table[i];
                                calls->LockMem();
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
                        if(nids[i] == nid) {
                                DEBUG_LOG_("Match found!");
                                *functionPtrLocation = (SceUInt)entryTable[i];
                                calls->LockMem();
                                return 0;
                        }
                }
        }

        calls->LockMem();
        DEBUG_LOG("Failed to find import NID 0x%08x in imports for %s", nid, libName);

        return -1;
}

int nid_table_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls)
{
        nidTable_entry libKernelBase;
        DEBUG_LOG_("Resolving VHL Imports");
        if(nid_table_analyzeStub(ctx->libkernel_anchor, 0, &libKernelBase) >= 0) {

                DEBUG_LOG_("Initializing cache");
                nid_storage_initialize(calls);

                if(nid_table_addNIDCacheToTable(calls, libKernelBase.value.function) < 0) return -1;

                libKernelBase.value.location = B_UNSET(libKernelBase.value.location, 0);
                int err = nid_table_resolveImportFromNID(calls,(SceUInt*)&calls->sceKernelGetModuleList,
                                                         SCE_KERNEL_GET_MODULE_LIST,
                                                         libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetModuleInfo,
                                                     SCE_KERNEL_GET_MODULE_INFO,
                                                     libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelLoadModule,
                                                     SCE_KERNEL_LOADMODULE,
                                                     libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelUnloadModule,
                                                     SCE_KERNEL_UNLOADMODULE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelStopUnloadModule,
                                                     SCE_KERNEL_STOP_UNLOADMODULE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                DEBUG_LOG_("Resolving and Caching NIDs...");
                nid_table_resolveAll(calls);

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelLoadStartModule,
                                                     SCE_KERNEL_LOAD_STARTMODULE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOMkdir,
                                                     SCE_IO_MKDIR,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelAllocMemBlock,
                                                     SCE_KERNEL_ALLOC_MEMBLOCK,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetMemBlockBase,
                                                     SCE_KERNEL_GET_MEMBLOCK_BASE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIORead,
                                                     SCE_IO_READ,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOWrite,
                                                     SCE_IO_WRITE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOLseek,
                                                     SCE_IO_LSEEK,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelFindMemBlockByAddr,
                                                     SCE_KERNEL_FIND_MEMBLOCK_BY_ADDR,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelFreeMemBlock,
                                                     SCE_KERNEL_FREE_MEMBLOCK,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOOpen,
                                                     SCE_IO_OPEN,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOWrite,
                                                     SCE_IO_WRITE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceIOClose,
                                                     SCE_IO_CLOSE,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelCreateThread,
                                                     SCE_KERNEL_CREATE_THREAD,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelStartThread,
                                                     SCE_KERNEL_START_THREAD,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetThreadId,
                                                     SCE_KERNEL_GET_THREAD_ID,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelDelayThread,
                                                     SCE_KERNEL_DELAY_THREAD,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelDeleteThread,
                                                     SCE_KERNEL_DELETE_THREAD,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelExitDeleteThread,
                                                     SCE_KERNEL_EXIT_DELETE_THREAD,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelGetThreadInfo,
                                                     SCE_KERNEL_GET_THREAD_INFO,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nid_table_resolveImportFromNID(calls, (SceUInt*)&calls->sceKernelWaitThreadEnd,
                                                     SCE_KERNEL_WAIT_THREAD_END,
                                                     libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;


                return 0;
        }
        return -1;
}

int nid_table_exportFunc(VHLCalls *calls, void *target, SceNID nid)
{
        calls->UnlockMem();

        nidTable_entry entry;
        entry.nid = nid;
        entry.type = ENTRY_TYPES_FUNCTION;
        entry.value.location = (SceUInt)target;
        entry.stub_loc = target;

        nid_storage_addEntry(calls, &entry);

        calls->LockMem();
        return 0;
}

int nid_table_resolveStub(VHLCalls *calls, int priority, void *stub, SceNID nid)
{
        nidTable_entry entry;

        int result = 0;
        if(priority > 0) result = nid_storage_getHookEntry(nid, &entry);
        if(priority <= 0 || result < 0) result = nid_storage_getEntry(nid, &entry);

        if(result >= 0) {
                //Fill the stub with the code generated for this entry
                ARM_INSTRUCTION movt;
                ARM_INSTRUCTION movw;
                ARM_INSTRUCTION jmp;

                stub = (void*)((SceUInt)stub & ~1);

                switch(entry.type) {
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
                        movt.value[1] = (SceUInt16)((SceUInt)entry.stub_loc >> 16);                          //Only the top part

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
        DEBUG_LOG("Failed to find NID 0x%08x", nid);
        return -1;
}

int nid_table_registerHook(VHLCalls *calls, void *func, SceNID nid, void **target)
{
        nidTable_entry entry;
        if(nid_storage_getEntry(nid, &entry) >= 0)
        {
                *target = entry.stub_loc;
                entry.stub_loc = func;
                nid_storage_addHookEntry(calls, &entry);
                return 0;
        }
        return -1;
}
