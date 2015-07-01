#ifndef _VHL_NIDCACHE_H_
#define  _VHL_NIDCACHE_H_

#include "vhl.h"

typedef struct{
      char *moduleName;
      SceNID moduleNID;
      SceNID* func_nids;
      SceUInt func_nid_count;
} NIDCache;

int GetNIDCache(SceNID moduleNID, NIDCache *cache);

#endif
