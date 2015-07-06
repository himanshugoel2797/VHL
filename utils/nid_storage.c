#include "nid_storage.h"

static nidTable_entry nid_storage_table[NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES];
static nidTable_entry nid_storage_table_hooks[MAX_SLOTS][NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES];

int nid_storage_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        for(int i = 0; i < NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                nid_storage_table[i].nid = 0;
        }
        calls->LockMem();
        return 0;
}

int nid_storage_addEntry(VHLCalls *calls, nidTable_entry *entry)
{
        int key = (char)(entry->nid >> 24);
        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES); i < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                if(nid_storage_table[i].nid == 0 || nid_storage_table[i].nid == entry->nid) { //Search for empty spot to add entry or update duplicate
                        calls->UnlockMem();
                        nid_storage_table[i].nid = entry->nid;
                        nid_storage_table[i].stub_loc = entry->stub_loc;
                        nid_storage_table[i].type = entry->type;
                        nid_storage_table[i].value.location = entry->value.location;
                        calls->LockMem();
                        return 0;
                }
        }
        return -1;
}

int nid_storage_getEntry(SceNID nid, nidTable_entry *entry)
{
        int key = (char)(nid >> 24);
        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES); i < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                if(nid_storage_table[i].nid == nid) {
                        entry->nid = nid_storage_table[i].nid;
                        entry->stub_loc = nid_storage_table[i].stub_loc;
                        entry->type = nid_storage_table[i].type;
                        entry->value.location = nid_storage_table[i].value.location;
                        return 0;
                }
        }
        return -1;
}

int nid_storage_addHookEntry(VHLCalls *calls, nidTable_entry *entry)
{

}
