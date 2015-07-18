/*
   main.c : Initializes the loader and launches the menu
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
#include <psp2/types.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include <hook/threadmgr.h>
#include <utils/bithacks.h>
#include <vhl.h>
#include <nid_table.h>
#include <nidcache.h>
#include <elf_parser.h>
#include <stub.h>

static globals_t *globals;

static int loadMenu()
{
        int res;

        globals->isMenu = 1;

        DEBUG_PUTS("Loading menu");
        res = elf_parser_load(globals->allocatedBlocks, "pss0:/top/Documents/homebrew.self", NULL);
        if (res) {
                internal_printf("Load failed 0x%08X", res);
                return res;
        }

        DEBUG_PUTS("Launching");
        return elf_parser_start(globals->allocatedBlocks);
}

static int exitCb(int notifyId __attribute__((unused)),
           int notifyCount __attribute__((unused)),
           int notifyArg,
           void *common __attribute__((unused)))
{
        DEBUG_PRINTF("Exited 0x%08X", notifyArg);

        if (globals->isMenu)
                sceKernelExitProcess(notifyArg);

        terminateDeleteAllUserThreads();
        return loadMenu();
}

static int loadExecCb()
{
        int res;

        DEBUG_PUTS("Killing all user threads");
        terminateDeleteAllUserThreads();
        globals->isMenu = 0;

        DEBUG_PUTS("Loading homebrew");
        res = elf_parser_load(globals->allocatedBlocks, globals->loadExecPath, NULL);
        if (res) {
                internal_printf("Load failed 0x%08X", res);
                loadMenu();
                return res;
        }

        DEBUG_PUTS("Launching");
        res = elf_parser_start(globals->allocatedBlocks);
        if (res) {
                internal_printf("Start failed 0x%08X", res);
                sceKernelExitProcess(res);
        }

        return res;
}

int __attribute__ ((section (".text.start")))
_start(UVL_Context *ctx)
{
        void * const vhlStubTop = getVhlStubTop();
        void * const vhlPrimaryStubTop = (void *)((uintptr_t)vhlStubTop + 16);
        void * const vhlPrimaryStubBtm = (void *)((uintptr_t)vhlPrimaryStubTop + vhlPrimaryStubSize);
        SceUInt timeout = 16384;

        const SceModuleImports * cachedImports[CACHED_IMPORTED_MODULE_NUM];
        nidTable_entry libkernelBase;
        SceModuleInfo *libkernelInfo;
        SceUID uid;
        void *p;
        int err;

        ctx->logline("Starting VHL...");

        /* It may get wrong if you change the order of nid_table_resolveVhl*
           See stub.S to know what imports will be resolved with those functions. */
        ctx->logline("Resolving VHL puts");
        nid_table_resolveVhlPuts(vhlStubTop, ctx);
        ctx->psvFlushIcache(vhlStubTop, 16);

        DEBUG_PUTS("Searching for SceLibKernel");
        if (nid_table_analyzeStub(ctx->libkernel_anchor, 0, &libkernelBase) != ANALYZE_STUB_OK) {
                DEBUG_PUTS("Failed to find the base of SceLibKernel");
                return -1;
        }

        libkernelBase.value.i = B_UNSET(libkernelBase.value.i, 0);

        libkernelInfo = nid_table_findModuleInfo(libkernelBase.value.p, KERNEL_MODULE_SIZE, "SceLibKernel");
        if (libkernelInfo == NULL) {
                DEBUG_PUTS("Failed to find the module information of SceLibKernel");
                return -1;
        }

        DEBUG_PUTS("Searching for cached imports");
        nidCacheFindCachedImports(libkernelInfo, cachedImports);

        DEBUG_PUTS("Resolving VHL primary imports");
        nid_table_resolveVhlPrimaryImports(vhlPrimaryStubTop, vhlPrimaryStubSize,
                                          libkernelInfo, cachedImports, ctx);
        ctx->psvFlushIcache(vhlPrimaryStubTop, vhlPrimaryStubSize);

        DEBUG_PUTS("Allocating memory for VHL");
        uid = sceKernelAllocMemBlock("vhlGlobals", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
                                     FOUR_KB_ALIGN(sizeof(globals_t)), NULL);
        if (uid < 0) {
                DEBUG_PRINTF("Failed to allocate memory block 0x%08X", uid);
                return uid;
        }

        err = sceKernelGetMemBlockBase(uid, &p);
        if (err < 0) {
                DEBUG_PRINTF("Failed to retrive memory block 0x%08X", err);
                return uid;
        }

        ctx->psvUnlockMem();
        globals = p;
        ctx->psvLockMem();

        DEBUG_PUTS("Initializing table");
        nid_storage_initialize();

        DEBUG_PUTS("Searching and Adding stubs to table...");
        nid_table_addAllStubs();

        DEBUG_PUTS("Resolving VHL secondary imports");
        nid_table_resolveVhlSecondaryImports(vhlPrimaryStubBtm, vhlSecondaryStubSize,
                                          libkernelInfo, cachedImports, ctx);
        ctx->psvFlushIcache(vhlPrimaryStubBtm, vhlSecondaryStubSize);

        DEBUG_PUTS("Adding stubs to table with cache");
        if (nid_table_addNIDCacheToTable(cachedImports) < 0)
                return -1;

        DEBUG_PUTS("Adding hooks to table");
        nid_table_addAllHooks();

        //TODO find a way to free unused memory

        DEBUG_PUTS("Initializing variables");
        block_manager_initialize();  //Initialize the elf block slots
        config_initialize();
        initThreadmgr();

        globals->exitCb = sceKernelCreateCallback("vhlExitCb", 0, exitCb, NULL);
        globals->loadExecCb = sceKernelCreateCallback("vhlLoadExecCb", 0, loadExecCb, NULL);

        loadMenu();

        DEBUG_PUTS("Menu loaded");
        while(1) {
                //Delay thread and check for flags and things to update every once in a while, check for exit combination
                err = waitAllUserThreadsEndCB(&timeout);
                if (err)
                        continue;

                if (globals->isMenu)
                        sceKernelExitProcess(0);

                loadMenu();
        }

        return 0;
}

globals_t *getGlobals()
{
        return globals;
}
