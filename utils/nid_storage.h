#ifndef _VHL_NID_STORAGE_H_
#define _VHL_NID_STORAGE_H_

#include <psp2/types.h>
#include "../vhl.h"
#include "../config.h"

#define NID_STORAGE_BUCKET_COUNT 256
#define NID_STORAGE_CACHE_FILE VHL_DATA_PATH"/nidCache.bin"


typedef enum  {
        ENTRY_TYPES_UNKN = 0,
        ENTRY_TYPES_FUNCTION,
        ENTRY_TYPES_SYSCALL,
        ENTRY_TYPES_VARIABLE,
        ENTRY_TYPES_RELOC,
}EntryTypes;

//Represents an entry in the NID table
typedef struct {
        SceNID nid;
        int type;
        void *stub_loc;
        union {
                void *function;
                SceUInt location;
        } value;
} nidTable_entry;

int nid_storage_initialize(VHLCalls *calls);
int nid_storage_addEntry(VHLCalls *calls, nidTable_entry *entry);
int nid_storage_getEntry(SceNID nid, nidTable_entry *entry);
int nid_storage_addHookEntry(VHLCalls *calls, nidTable_entry *entry);

#endif
