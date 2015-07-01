#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"
#include "nidcache.h"

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

        calls.LogLine("Initializing VHL...");
        calls.LogLine("Bootstrapping...");
        nidCacheInitialize(&calls, (SceUInt)&_start);

        int err = nidTable_resolveVHLImports(ctx, &calls);
        if(err < 0) {
                calls.LogLine("Failed to resolve some functions... VHL might not work...");
        }

        calls.LogLine("Resolving and Caching NIDs...");

        nidTable_resolveAll(&calls);

        calls.LogLine("Freeing memory...");

        calls.LogLine("Loading plugins...");

        calls.LogLine("Loading menu...");

        calls.LogLine("Launching menu...");

        while(1) {

                calls.LogLine("Menu exited! Relaunching...");
        }

        return 0;
}

void logLine(const char *str)
{
        calls.LogLine(str);
}
