#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/memorymgr.h>
#include "vhl.h"
#include "nid_table.h"


int _start(UVL_Context *ctx)
{
        VHLCalls calls;
        nidTable_resolveVHLImports(ctx, &calls);
}
