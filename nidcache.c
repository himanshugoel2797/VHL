#include "nidcache.h"

static SceUInt libkernel_nid_cache_header;
static SceUInt libkernel_nid_cache;

int nidCacheInitialize(VHLCalls *calls){
        calls->UnlockMem();
        libkernel_nid_cache_header = ( calls->loadAddress + (SceUInt)libkernel_nid_cache_header_);
        libkernel_nid_cache = ( calls->loadAddress + (SceUInt)libkernel_nid_cache_);
        calls->LockMem();
        return 0;
}

int nidCacheContainsModuleNID(SceNID nid, int *offset)
{
        int off = 0;
        NID_CACHE *header = (NID_CACHE*)libkernel_nid_cache_header;
        for(int i = 0; header[i].module_nid != 0; i++) {
                if(header[i].module_nid == nid) {
                        *offset = off;
                        return i;
                }
                off += header[i].count;
        }
        return -1;
}

NID_CACHE* nidCache_getHeader(){
        return (NID_CACHE*)libkernel_nid_cache_header;
}
SceNID* nidCache_getCache(){
        return (SceNID*)libkernel_nid_cache;
}
