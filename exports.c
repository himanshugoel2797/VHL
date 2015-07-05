#include "exports.h"

static VHLCalls func_calls;

static int export_printf(const char* fmt, ...)
{
        char buffer[INTERNAL_PRINTF_MAX_LENGTH];
        va_list va;
        va_start(va, fmt);
        mini_vsnprintf(buffer, INTERNAL_PRINTF_MAX_LENGTH, fmt, va);
        va_end(va);
        logLine(buffer);
        return 0;
}

static SceUID allocCodeMem(int size)
{
        return AllocCodeMemBlock(size);
}

int exports_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        func_calls.AllocCodeMem = calls->AllocCodeMem;
        func_calls.FlushICache = calls->FlushICache;
        func_calls.UnlockMem = calls->UnlockMem;
        func_calls.LockMem = calls->LockMem;
        func_calls.LogLine = calls->LogLine;
        func_calls.sceKernelAllocMemBlock = calls->sceKernelAllocMemBlock;
        func_calls.sceKernelFreeMemBlock = calls->sceKernelFreeMemBlock;
        func_calls.sceKernelGetMemBlockBase = calls->sceKernelGetMemBlockBase;
        func_calls.sceKernelFindMemBlockByAddr = calls->sceKernelFindMemBlockByAddr;
        calls->LockMem();

        nidTable_exportFunc(calls, &allocCodeMem, ALLOC_CODE_MEM);
        nidTable_exportFunc(calls, &export_printf, PRINTF);
        nidTable_exportFunc(calls, func_calls.LogLine, PUTS);
        nidTable_exportFunc(calls, func_calls.LogLine, LOG);
        nidTable_exportFunc(calls, func_calls.UnlockMem, UNLOCK);
        nidTable_exportFunc(calls, func_calls.LockMem, LOCK);
        nidTable_exportFunc(calls, func_calls.FlushICache, FLUSH);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return func_calls.sceKernelFindMemBlockByAddr(func_calls.AllocCodeMem(&size), 0);
}

//TODO export module functions so they can be called from other threads
