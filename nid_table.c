#include "nid_table.h"

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
        //Check if an SceModuleInfo struct is valid

        return 1;
}

int nidTable_resolveAll()
{

}

int nidTable_resolveImportFromNID(SceUInt *functionPtrLocation, SceNID nid, void *libraryBase, char* libName)
{
        //Find the module info struct
        SceModuleInfo *moduleInfo = (SceModuleInfo*)memstr((char*)libraryBase, KERNEL_MODULE_SIZE, libName, strlen(libName));
        if(nidTable_isValidModuleInfo(moduleInfo)) {
                moduleInfo = (SceModuleInfo*)((SceUInt)moduleInfo - 4);
                SceUInt base = (SceUInt)moduleInfo - moduleInfo->ent_top + sizeof(SceModuleInfo);
                SceModuleExports *exportTable = (SceModuleExports*)(base + moduleInfo->ent_top);
                SceModuleImports *importTable = (SceModuleImports*)(base + moduleInfo->stub_top);

                //Check the export tables to see if the NID is present
                //If so assign the respective pointer to functionPtrLocation

                //Ceck the import tables to see if the NID is present
                //If so assign the respective pointer to functionPtrLocation
                //Else exit with error code -1
        }
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
                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleList,
                                              SCE_KERNEL_GET_MODULE_LIST,
                                              libKernelBase.value.function, "SceLibKernel");

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleInfo,
                                              SCE_KERNEL_GET_MODULE_INFO,
                                              libKernelBase.value.function,"SceLibKernel");

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadModule,
                                              SCE_KERNEL_LOADMODULE,
                                              libKernelBase.value.function,"SceLibKernel");

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadStartModule,
                                              SCE_KERNEL_LOAD_STARTMODULE,
                                              libKernelBase.value.function, "SceLibKernel");

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelStopUnloadModule,
                                              SCE_KERNEL_STOP_UNLOADMODULE,
                                              libKernelBase.value.function, "SceLibKernel");

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelUnloadModule,
                                              SCE_KERNEL_UNLOADMODULE,
                                              libKernelBase.value.function, "SceLibKernel");

                return 0;
        }
        return -1;
}
