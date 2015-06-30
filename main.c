#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"

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

        nidTable_resolveVHLImports(ctx, &calls);

        calls.LogLine("Resolving and Caching NIDs...");

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
