#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"
#include "nidcache.h"
#include "elf_parser.h"
#include "exports.h"
#include "hbrew_resource_manager/resource_manager.h"

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

        for(int i = 0; i < 3; i++) {
                if(i > 0) calls.LogLine("Retrying...");

                int err = nid_table_resolveVHLImports(ctx, &calls);
                if(err < 0) {
                        calls.LogLine("Failed to resolve some functions... VHL will not work...");
                        return -1;
                }else{
                        break;
                }
        }

        DEBUG_LOG_("Freeing memory...");
        //TODO
        block_manager_initialize(&calls);  //Initialize the elf block slots

        DEBUG_LOG_("Loading plugins...");
        //TODO

        exports_initialize(&calls);
        config_initialize(&calls);
        loader_initialize(&calls);
        resource_manager_initialize(&calls);

        DEBUG_LOG_("Loading menu...");

        if(elf_parser_load(&calls, 1, 0, "pss0:/top/Documents/homebrew.self", NULL) < 0) {
                internal_printf("Load failed!");
                return -1;
        }
        internal_printf("Load succeeded! Launching!");
        elf_parser_start(&calls, 0);

        while(1) {
                //Delay thread and check for flags and things to update every once in a while, check for exit combination
                //calls.LogLine("Menu exited! Relaunching...");
        }

        return 0;
}

void logLine(const char *str)
{
        calls.LogLine(str);
}
