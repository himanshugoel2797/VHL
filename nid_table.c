#include "nid_table.h"

static int currentIndex = 0;
static nidTable_entry entries[NID_TABLE_MAX_ENTRIES];

int resolveStub(void *stub, SceNID nid, nidTable_entry *entry)
{
        internal_printf("stub");

        entry->nid = nid;
        entry->type = ENTRY_TYPES_UNKN;
        entry->value.location = 0;

        ARM_INSTRUCTION instr;

        while(entry->type == ENTRY_TYPES_UNKN | entry->value.location == 0)
        {
                DEBUG_LOG("stub 0x%08x", stub);
                if(Disassemble(stub, &instr) >= 0 && instr.type != ARM_MVN_INSTRUCTION)
                {
                        DEBUG_LOG("Conditions: %08x", instr.condition);
                        DEBUG_LOG("Instruction: %08x", instr.instruction);
                        DEBUG_LOG("Type: %08x", instr.type);
                        if(instr.argCount > 0) DEBUG_LOG("Value[0]: %08x", instr.value[0]);
                        switch(instr.instruction)
                        {
                        case ARM_INST_MOVW:
                                entry->value.location = instr.value[1];
                                DEBUG_LOG_("MOVW");
                                break;
                        case ARM_INST_MOVT:
                                entry->value.location |= instr.value[1] << 16;
                                DEBUG_LOG_("MOVT");
                                break;

                        case ARM_INST_BX:
                        case ARM_INST_BLX:
                                entry->type = ENTRY_TYPES_FUNCTION;
                                DEBUG_LOG_("BRANCH");
                                break;
                        case ARM_INST_SVC:
                                entry->type = ENTRY_TYPES_SYSCALL;
                                DEBUG_LOG_("SVC");
                                break;
                        //TODO Need to figure out what involves a relative stub, what is it relative to?
                        default:
                                DEBUG_LOG_("ERROR");
                                break;
                        }
                }
                stub = (void*)((SceUInt)stub + 4);
                DEBUG_LOG_("Trying again");
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

int nidTable_resolveAll(VHLCalls *calls)
{
        SceUID uids[NID_TABLE_MAX_MODULES];
        SceUInt numEntries = NID_TABLE_MAX_ENTRIES;

        int err = calls->sceKernelGetModuleList(0xFF, uids, &numEntries);
        if(err < 0) {
                internal_printf("Failed to get module list... Exiting...");
                return -1;
        }
        SceLoadedModuleInfo loadedModuleInfo;

        for(int i = 0; i < numEntries; i++)
        {
                loadedModuleInfo.size = sizeof(loadedModuleInfo);
                if(calls->sceKernelGetModuleInfo(uids[i], &loadedModuleInfo) < 0) {
                        internal_printf("Failed to get module info... Skipping...");
                }else{
                        SceModuleInfo *moduleInfo = nidTable_findModuleInfo(loadedModuleInfo.segments[0].vaddr, loadedModuleInfo.segments[0].memsz, loadedModuleInfo.module_name);
                        if(moduleInfo != NULL) {
                                internal_printf("Module Found: %s", moduleInfo->modname);
                        }
                }
        }
}

SceModuleInfo* nidTable_findModuleInfo(void* location, SceUInt size, char* libname)
{
        DEBUG_LOG("Looking for SceModuleInfo for %s", libname);
        SceModuleInfo *moduleInfo = NULL;
        void *origLocation = location;
        //Find the module info string in this memory region
        while(size > 0)
        {
                location = memstr(location, size, libname, strlen(libname));
                if(location == NULL)
                {
                        internal_printf("Failed to find module info");
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

#ifdef DEBUG
        if(moduleInfo != NULL) DEBUG_LOG("Found SceModuleInfo at 0x%08x", moduleInfo);
#endif
        return moduleInfo;
}

int nidTable_resolveImportFromNID(SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName)
{
        //Find the module info struct
        SceModuleInfo *moduleInfo = nidTable_findModuleInfo(libraryBase, KERNEL_MODULE_SIZE, libName);
        SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);
        SceModuleExports *exportTable = (SceModuleExports*)(base + moduleInfo->ent_top);
        SceModuleImports *importTable = (SceModuleImports*)(base + moduleInfo->stub_top);

        DEBUG_LOG("Searching for NID 0x%08x in module %s", nid, libName);

        //Check the export tables to see if the NID is present
        //If so assign the respective pointer to functionPtrLocation
        for(; (SceUInt)exportTable < (SceUInt)(base + moduleInfo->ent_end); exportTable++) {
                DEBUG_LOG("Export Table at 0x%08x", exportTable);
                for(int i = 0; i < exportTable->num_functions; i++)
                {
                        if(exportTable->nid_table[i] == nid) {
                                //Found the nid
                                DEBUG_LOG_("Match found!");
                                *functionPtrLocation = exportTable->entry_table[i];
                                return 0;
                        }
                }
        }
        DEBUG_LOG("Failed to find import NID 0x%08x in exports for %s", nid, libName);

        for(; (SceUInt)importTable < (SceUInt)(base + moduleInfo->stub_end); importTable++) {
                DEBUG_LOG("Import Table at 0x%08x", importTable);
                for(int i = 0; i < SCE_MODULE_IMPORTS_GET_FUNCTION_COUNT(importTable); i++)
                {
                        if(SCE_MODULE_IMPORTS_GET_FUNCTIONS_NIDTABLE(importTable)[i] == nid) {
                                *functionPtrLocation = SCE_MODULE_IMPORTS_GET_FUNCTIONS_ENTRYTABLE(importTable)[i];
                                return 0;
                        }
                }
        }
        DEBUG_LOG("Failed to find import NID 0x%08x in imports for %s", nid, libName);


        return -1;
}


int nidTable_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls)
{
//Get libkernel base address
//Search for libkernel module info
//Unload unloadable modules
//Resolve all the NIDs

        nidTable_entry libKernelBase;
        internal_printf("Resolving VHL Imports");
        if(resolveStub(ctx->libkernel_anchor, 0, &libKernelBase) >= 0) {
                internal_printf("libKernelBase found at 0x%08x", libKernelBase.value.location);
                B_UNSET(libKernelBase.value.location, 1);
                int err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleList,
                                                        SCE_KERNEL_GET_MODULE_LIST,
                                                        libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleInfo,
                                                    SCE_KERNEL_GET_MODULE_INFO,
                                                    libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadModule,
                                                    SCE_KERNEL_LOADMODULE,
                                                    libKernelBase.value.function,"SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadStartModule,
                                                    SCE_KERNEL_LOAD_STARTMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelStopUnloadModule,
                                                    SCE_KERNEL_STOP_UNLOADMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                err = nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelUnloadModule,
                                                    SCE_KERNEL_UNLOADMODULE,
                                                    libKernelBase.value.function, "SceLibKernel");
                if(err < 0) return -1;

                return 0;
        }
        return -1;
}
