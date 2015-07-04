#include "exports.h"

static VHLCalls func_calls;

int exports_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        func_calls.AllocCodeMem = calls->AllocCodeMem;
        func_calls.FlushICache = calls->FlushICache;
        func_calls.LockMem = calls->LockMem;
        func_calls.LogLine = calls->LogLine;
        func_calls.sceKernelAllocMemBlock = calls->sceKernelAllocMemBlock;
        func_calls.sceKernelFreeMemBlock = calls->sceKernelFreeMemBlock;
        func_calls.sceKernelGetMemBlockBase = calls->sceKernelGetMemBlockBase;
        func_calls.sceKernelFindMemBlockByAddr = calls->sceKernelFindMemBlockByAddr;
        calls->LockMem();

        nidTable_exportFunc(calls, AllocCodeMemBlock, ALLOC_CODE_MEM);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return func_calls.sceKernelFindMemBlockByAddr(func_calls.AllocCodeMem(&size), 0);
}

//TODO export module functions so they can be called from other threads
