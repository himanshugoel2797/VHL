#include "nidcache.h"

static SceUInt libkernel_nid_cache_header;
static SceUInt libkernel_nid_cache;

int nidCacheInitialize(VHLCalls *calls, SceUInt base){
        calls->UnlockMem();
        libkernel_nid_cache_header = (base - 1 + (SceUInt)libkernel_nid_cache_header_);
        libkernel_nid_cache = (base - 1 + (SceUInt)libkernel_nid_cache_);
        calls->LockMem();
        DEBUG_LOG("start: 0x%08x", base);
        DEBUG_LOG("header: 0x%08x", libkernel_nid_cache_header);
}

int nidCacheContainsModuleNID(SceNID nid, int *offset)
{
        int off = 0;
        NID_CACHE *header = (NID_CACHE*)libkernel_nid_cache_header;

        DEBUG_LOG("libkernel_nid_cache_header location 0x%08x", libkernel_nid_cache_header);
        for(int i = 0; header[i].module_nid != 0; i++) {
                DEBUG_LOG("NID: 0x%08x", header[i].module_nid);
                if(header[i].module_nid == nid) {
                        *offset = off;
                        return i;
                }
                off += header[i].count;
        }
        DEBUG_LOG_("Fail");
        return -1;
}

NID_CACHE* nidCache_getHeader(){
        return (NID_CACHE*)libkernel_nid_cache_header;
}
SceNID* nidCache_getCache(){
        return (SceNID*)libkernel_nid_cache;
}
