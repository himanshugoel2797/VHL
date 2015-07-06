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

int homebrew_load(const char *path, int slot)
{
  export_printf("%s", path);
  int (*load)(VHLCalls*, int, char*, void**) = ((SceUInt)&elfParser_Load + func_calls.loadAddress);   //Offset is correct, jump still crashes
  export_printf("0x%08x", &elfParser_Load);
  export_printf("0x%08x", &load);
  return load(&func_calls, slot, path, NULL);
}

static int homebrew_start(int slot)
{
  export_printf("Starting...");
  return elfParser_Start(&func_calls, slot);
}

static int homebrew_pause(int slot)
{
  return -1;
}

static int homebrew_stop(int slot)
{
  return -1;
}

static SceUID allocCodeMem(int size)
{
        return AllocCodeMemBlock(size);
}

int exports_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        func_calls.loadAddress = calls->loadAddress;
        func_calls.AllocCodeMem = calls->AllocCodeMem;
        func_calls.FlushICache = calls->FlushICache;
        func_calls.UnlockMem = calls->UnlockMem;
        func_calls.LockMem = calls->LockMem;
        func_calls.LogLine = calls->LogLine;
        func_calls.sceKernelAllocMemBlock = calls->sceKernelAllocMemBlock;
        func_calls.sceKernelFreeMemBlock = calls->sceKernelFreeMemBlock;
        func_calls.sceKernelGetMemBlockBase = calls->sceKernelGetMemBlockBase;
        func_calls.sceKernelFindMemBlockByAddr = calls->sceKernelFindMemBlockByAddr;
        func_calls.sceIOOpen = calls->sceIOOpen;
        func_calls.sceIORead = calls->sceIORead;
        func_calls.sceIOLseek = calls->sceIOLseek;
        func_calls.sceIOClose = calls->sceIOClose;
        calls->LockMem();

        nidTable_exportFunc(calls, &allocCodeMem, ALLOC_CODE_MEM);
        nidTable_exportFunc(calls, &export_printf, PRINTF);
        nidTable_exportFunc(calls, func_calls.LogLine, PUTS);
        nidTable_exportFunc(calls, func_calls.LogLine, LOG);
        nidTable_exportFunc(calls, func_calls.UnlockMem, UNLOCK);
        nidTable_exportFunc(calls, func_calls.LockMem, LOCK);
        nidTable_exportFunc(calls, func_calls.FlushICache, FLUSH);
        nidTable_exportFunc(calls, &homebrew_load, LOAD_ELF);
        nidTable_exportFunc(calls, &homebrew_start, START_ELF);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return func_calls.sceKernelFindMemBlockByAddr(func_calls.AllocCodeMem(&size), 0);
}

//TODO export module functions so they can be called from other threads
