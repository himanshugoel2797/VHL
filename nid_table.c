#include "nid_table.h"

int resolveStub(void *stub, SceNID nid, nidTable_entry *entry)
{
        entry->nid = nid;
        entry->type = ENTRY_TYPES_UNKN;
        entry->value.location = 0;

        ARM_INSTRUCTION instr;

        while(entry->type == ENTRY_TYPES_UNKN)
        {
                if(Disassemble(stub, &instr) >= 0 && instr.type != ARM_MVN_INSTRUCTION)
                {
                        switch(instr.instruction)
                        {
                        case ARM_INST_MOVW:
                                entry->value.location = instr.value[0];
                                break;
                        case ARM_INST_MOVT:
                                entry->value.location |= instr.value[0] << 16;
                                break;

                        case ARM_INST_BX:
                        case ARM_INST_BLX:
                                entry->type = ENTRY_TYPES_FUNCTION;
                                break;
                        case ARM_INST_SVC:
                                entry->type = ENTRY_TYPES_SYSCALL;
                                break;
                        //TODO Need to figure out what involves a relative stub, what is it relative to?
                        default:
                                break;
                        }
                }
                stub = (void*)((SceUInt*)stub + sizeof(SceUInt));
        }
        return 0;
}

int nidTable_resolveAll()
{

}

int nidTable_resolveImportFromNID(SceUInt *functionPtrLocation, void *libraryBase, SceNID nid, const char* libName)
{
        //Find the module info struct
        SceModuleInfo *moduleInfo = (SceModuleInfo*)memstr((char*)libraryBase, KERNEL_MODULE_SIZE, libName, strlen(libName));
        moduleInfo = (SceModuleInfo*)((SceUInt)moduleInfo - 4);


}


int nidTable_resolveVHLImports(UVL_Context *ctx, VHLCalls *calls)
{
//Get libkernel base address
//Search for libkernel module info
//Unload unloadable modules
//Resolve all the NIDs
        nidTable_entry libKernelBase;
        if(resolveStub(ctx->libkernel_anchor, 0, &libKernelBase) >= 0) {
                B_UNSET(libKernelBase.value.location, 1);
                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleList,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_GET_MODULE_LIST);

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelGetModuleInfo,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_GET_MODULE_INFO);

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadModule,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_LOADMODULE);

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelLoadStartModule,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_LOAD_STARTMODULE);

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelStopUnloadModule,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_STOP_UNLOADMODULE);

                nidTable_resolveImportFromNID((SceUInt*)&calls->sceKernelUnloadModule,
                                              libKernelBase.value.function,
                                              SCE_KERNEL_UNLOADMODULE);

                return 0;
        }
        return -1;
}
