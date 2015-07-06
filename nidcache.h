#ifndef _VHL_NIDCACHE_H_
#define _VHL_NIDCACHE_H_

#include "vhl.h"

typedef struct {
        SceNID module_nid;
        SceUInt count;
} NID_CACHE;

extern const NID_CACHE libkernel_nid_cache_header_[];
extern const SceNID libkernel_nid_cache_[];

int nidCacheInitialize(VHLCalls *calls);
int nidCacheContainsModuleNID(SceNID nid, int *offset);

NID_CACHE* nidCache_getHeader();
SceNID* nidCache_getCache();

#endif
