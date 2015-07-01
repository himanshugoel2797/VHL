#ifndef  _VHL_COMMON_H_
#define  _VHL_COMMON_H_

#include <psp2/types.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/memorymgr.h>
#include <psp2/kernel/threadmgr.h>
#include "mini-printf.h"
#include "elf_headers.h"

typedef SceUInt SceNID;


typedef struct {
        //IO Functions
        SceUID (*sceIOOpen)(const char*, int, SceMode);
        int (*sceIOClose)(SceUID);
        int (*sceIORead)(SceUID, void*, SceSize);
        int (*sceIOWrite)(SceUID, void*, SceSize);
        SceOff (*sceIOLseek)(SceUID, SceOff,int);
        int (*sceIOMkdir)(const char*, SceMode);
//Module management Functions
        int (*sceKernelGetModuleList)(int, SceUID*, int*);
        int (*sceKernelGetModuleInfo)(SceUID, SceLoadedModuleInfo*);
        SceUID (*sceKernelLoadModule)(const char* /*path*/, int /*flags*/, int* /*result*/);
        int (*sceKernelLoadStartModule)(SceUID, SceSize /*arg size*/, void* /*args*/, int /*flags*/, void* /*NULL*/, int* /*result*/);
        int (*sceKernelStopUnloadModule)(SceUID, SceSize /*arg size*/, void* /*args*/, int /*flags*/,void* /*NULL*/, int* /*result*/);
        int (*sceKernelUnloadModule)(SceUID);

//Memory management functions
        SceUID (*sceKernelAllocMemBlock)(const char*, int, int, void*);
        int (*sceKernelFreeMemBlock)(SceUID);
        int (*sceKernelGetMemBlockBase)(SceUID, void**);
        int (*sceKernelGetFreeMemorySize)(SceSize*);

        //UVL context calls
        void* (*AllocCodeMem)(SceUInt*);
        void (*UnlockMem)();
        void (*LockMem)();
        void (*FlushICache)(void*, SceUInt);
        int (*LogLine)(const char*);
} VHLCalls;

typedef struct {
        void* (*psvCodeAllocMem)(unsigned int *p_len); ///< Allocate code block
        void (*psvUnlockMem)(void);                ///< Unlock code block
        void (*psvLockMem)(void);                  ///< Relock code block
        void (*psvFlushIcache)(void *, unsigned int); ///< Flush Icache
        int (*logline)(const char *);              ///< Debug logging (optional)
        void *libkernel_anchor;                    ///< Any imported SceLibKernel function
} UVL_Context;

typedef enum
{
        SCE_IO_OPEN = 1818274913,
        SCE_IO_CLOSE = 3339421830,
        SCE_IO_READ = 4256375443,
        SCE_IO_WRITE = 888133750,
        SCE_IO_LSEEK = 2579109694,
        SCE_IO_MKDIR = 2523976607,

        SCE_KERNEL_GET_MODULE_LIST = 787634207,
        SCE_KERNEL_GET_MODULE_INFO = 911760815,
        SCE_KERNEL_LOADMODULE = 3152552277,
        SCE_KERNEL_LOAD_STARTMODULE = 768363258,
        SCE_KERNEL_STOP_UNLOADMODULE = 605419684,
        SCE_KERNEL_UNLOADMODULE = 428315150,

        SCE_KERNEL_ALLOC_MEMBLOCK = 3117804510,
        SCE_KERNEL_FREE_MEMBLOCK = 2837321198,
        SCE_KERNEL_GET_MEMBLOCK_BASE = 3102693400,
        SCE_KERNEL_GET_FREE_MEMORY_SIZE = 2278316043
}VHL_CALLS_NIDS;

int __attribute__ ((section (".text.start"))) _start(UVL_Context *ctx);
void logLine(const char *str);

#ifdef DEBUG
#define DEBUG_LOG(x, ...) internal_printf(x, __VA_ARGS__)
#define DEBUG_LOG_(x) internal_printf(x);
#else
#define DEBUG_LOG(...)
#define DEBUG_LOG_(x)
#endif

#endif
