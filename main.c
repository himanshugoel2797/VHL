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
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"
#include "nidcache.h"
#include "elf_parser.h"
#include "exports.h"
#include "state_machine.h"

static VHLCalls calls;

int __attribute__ ((section (".text.start")))
_start(UVL_Context *ctx)
{
        ctx->logline("Starting VHL...");

        //Initialize VHLCalls
        ctx->psvUnlockMem();
        calls.loadAddress = (SceUInt)&_start & ~1;
        calls.AllocCodeMem = ctx->psvCodeAllocMem;
        calls.UnlockMem = ctx->psvUnlockMem;
        calls.LockMem = ctx->psvLockMem;
        calls.FlushICache = ctx->psvFlushIcache;
        calls.LogLine = ctx->logline;
        ctx->psvLockMem();

        DEBUG_LOG_("Initializing VHL...");
        DEBUG_LOG_("Bootstrapping...");
        nidCacheInitialize(&calls);

        int err = nid_table_resolveVHLImports(ctx, &calls);
        if(err < 0) {
                calls.LogLine("Failed to resolve some functions... VHL will not work...");
                return -1;
        }

        //TODO find a way to free unused memory

        block_manager_initialize(&calls);  //Initialize the elf block slots

        //TODO decide how to handle plugins

        exports_initialize(&calls);
        config_initialize(&calls);
        loader_initialize(&calls);
        state_machine_initialize(&calls);

        DEBUG_LOG_("Loading menu...");

        if(elf_parser_load(&calls, 1, 0, "pss0:/top/Documents/homebrew.self", NULL) < 0) {
                internal_printf("Load failed!");
                return -1;
        }
        calls.LogLine("Load succeeded! Launching!");

        elf_parser_start(&calls, 0, 0);

        while(1) {
                //Delay thread and check for flags and things to update every once in a while, check for exit combination
                //calls.LogLine("Menu exited! Relaunching...");
                sceKernelDelayThread(16000);  //Update stuff once every 16 ms
        }

        return 0;
}

void logLine(const char *str)
{
        calls.LogLine(str);
}
