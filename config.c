#include "config.h"
#include "vhl.h"
#include "nid_table.h"
#include "nids.h"

static VHLCalls *calls;
static int intOptions[INT_VARIABLE_OPTION_COUNT];

int config_initialize(void *i)
{
        ((VHLCalls*)i)->UnlockMem();
        calls = ((VHLCalls*)i);
        for(int j = 0; j < INT_VARIABLE_OPTION_COUNT; j++) {
          intOptions[j] = 0;
        }
        ((VHLCalls*)i)->LockMem();

        nid_table_exportFunc(calls, config_getIntValue, GET_INT_VALUE);
        nid_table_exportFunc(calls, config_setIntValue, SET_INT_VALUE);

        return 0;
}

int config_getIntValue(INT_VARIABLE_OPTIONS option)
{
        return intOptions[option];
}

int config_setIntValue(INT_VARIABLE_OPTIONS option, int val)
{
        calls->UnlockMem();
        intOptions[option] = val;
        calls->LockMem();
        DEBUG_LOG("Option %08x Set to %08x ", option, val);
        return val;
}
