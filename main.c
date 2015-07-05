#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"
#include "nidcache.h"
#include "elf_parser.h"
#include "exports.h"

static VHLCalls calls;

int __attribute__ ((section (".text.start")))
_start(UVL_Context *ctx)
{
        ctx->logline("Starting VHL...");
        //Initialize VHLCalls
        ctx->psvUnlockMem();
        calls.AllocCodeMem = ctx->psvCodeAllocMem;
        calls.UnlockMem = ctx->psvUnlockMem;
        calls.LockMem = ctx->psvLockMem;
        calls.FlushICache = ctx->psvFlushIcache;
        calls.LogLine = ctx->logline;
        ctx->psvLockMem();

        DEBUG_LOG_("Initializing VHL...");
        DEBUG_LOG_("Bootstrapping...");
        nidCacheInitialize(&calls, (SceUInt)&_start);

        int err = nidTable_resolveVHLImports(ctx, &calls);
        if(err < 0) {
                calls.LogLine("Failed to resolve some functions... VHL will not work...");
                return -1;
        }

        DEBUG_LOG_("Freeing memory...");
        //TODO
        blockManager_initialize(&calls);  //Initialize the elf block slots

        DEBUG_LOG_("Loading plugins...");
        //TODO

        exports_initialize(&calls);
        DEBUG_LOG_("Loading menu...");

        int (*start)(int, char*);
        if(elfParser_Load(&calls, 0, "pss0:/top/Documents/homebrew.self", &start) < 0) {
                DEBUG_LOG_("Load failed!");
                return -1;
        }
        DEBUG_LOG_("Load succeeded! Launching!");
        DEBUG_LOG("0x%08x returned", start(0, NULL));

        while(1) {
                //calls.LogLine("Menu exited! Relaunching...");
        }

        return 0;
}

void logLine(const char *str)
{
        calls.LogLine(str);
}
