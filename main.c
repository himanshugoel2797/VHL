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
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include "utils/bithacks.h"
#include "vhl.h"
#include "nid_table.h"
#include "nidcache.h"
#include "elf_parser.h"
#include "state_machine.h"
#include "fs_hooks.h"
#include "loader.h"
#include "stub.h"

static globals_t *globals;

int __attribute__ ((section (".text.start")))
_start(UVL_Context *ctx)
{
        void * const vhlStubTop = getVhlStubTop();
        void * const vhlPrimaryStubTop = (void *)((uintptr_t)vhlStubTop + 16);
        void * const vhlPrimaryStubBtm = (void *)((uintptr_t)vhlPrimaryStubTop + vhlPrimaryStubSize);

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

        DEBUG_LOG_("Searching for SceLibKernel");
        if (nid_table_analyzeStub(ctx->libkernel_anchor, 0, &libkernelBase) != ANALYZE_STUB_OK) {
                DEBUG_LOG_("Failed to find the base of SceLibKernel");
                return -1;
        }

        libkernelBase.value.i = B_UNSET(libkernelBase.value.i, 0);

        libkernelInfo = nid_table_findModuleInfo(libkernelBase.value.p, KERNEL_MODULE_SIZE, "SceLibKernel");
        if (libkernelInfo == NULL) {
                DEBUG_LOG_("Failed to find the module information of SceLibKernel");
                return -1;
        }

        DEBUG_LOG_("Searching for cached imports");
        nidCacheFindCachedImports(libkernelInfo, cachedImports);

        DEBUG_LOG_("Resolving VHL primary imports");
        nid_table_resolveVhlPrimaryImports(vhlPrimaryStubTop, vhlPrimaryStubSize,
                                          libkernelInfo, cachedImports, ctx);
        ctx->psvFlushIcache(vhlPrimaryStubTop, vhlPrimaryStubSize);

        DEBUG_LOG_("Allocating memory for VHL");
        uid = sceKernelAllocMemBlock("vhlGlobals", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
                                     FOUR_KB_ALIGN(sizeof(globals_t)), NULL);
        if (uid < 0) {
                DEBUG_LOG("Failed to allocate memory block 0x%08X", uid);
                return uid;
        }

        err = sceKernelGetMemBlockBase(uid, &p);
        if (err < 0) {
                DEBUG_LOG("Failed to retrive memory block 0x%08X", err);
                return uid;
        }

        ctx->psvUnlockMem();
        globals = p;
        ctx->psvLockMem();

        DEBUG_LOG_("Initializing table");
        nid_storage_initialize();

        DEBUG_LOG_("Searching and Adding stubs to table...");
        nid_table_addAllStubs();

        DEBUG_LOG_("Resolving VHL secondary imports");
        nid_table_resolveVhlSecondaryImports(vhlPrimaryStubBtm, vhlSecondaryStubSize,
                                          libkernelInfo, cachedImports, ctx);
        ctx->psvFlushIcache(vhlPrimaryStubBtm, vhlSecondaryStubSize);

        DEBUG_LOG_("Adding stubs to table with cache");
        if (nid_table_addNIDCacheToTable(cachedImports) < 0)
                return -1;

        DEBUG_LOG_("Adding hooks to table");
        nid_table_addAllHooks();

        //TODO find a way to free unused memory

        block_manager_initialize();  //Initialize the elf block slots

        //TODO decide how to handle plugins

        config_initialize();

        DEBUG_LOG_("Loading menu...");

        if(elf_parser_load(globals->allocatedBlocks, "pss0:/top/Documents/homebrew.self", NULL) < 0) {
                internal_printf("Load failed!");
                return -1;
        }
        puts("Load succeeded! Launching!");

        elf_parser_start(globals->allocatedBlocks, 0);

        while(1) {
                //Delay thread and check for flags and things to update every once in a while, check for exit combination
                //calls.LogLine("Menu exited! Relaunching...");
                sceKernelDelayThread(16000);  //Update stuff once every 16 ms
        }

        return 0;
}

globals_t *getGlobals()
{
        return globals;
}
