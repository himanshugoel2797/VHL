/*
nidcahce.c : Provides access to the NID cache
Copyright (C) 2015  hgoel0974

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <stdint.h>
#if defined(PSV_3XX)
#include "nidcache3xx.c"
#endif
#include <nidcache.h>
#include <common.h>

void nidCacheFindCachedImports(const SceModuleInfo *libkernel,
                               const SceModuleImports *imports[CACHED_IMPORTED_MODULE_NUM])
{
        uintptr_t base = (intptr_t)libkernel - libkernel->ent_top + sizeof(SceModuleInfo);
        SceNID nid;
        unsigned int i;

        FOREACH_IMPORT(base, libkernel, importTable) {
                nid = GET_NID(importTable);

                for (i = 0; i < CACHED_IMPORTED_MODULE_NUM; i++)
                        if (libkernel_nid_cache_header[i].module_nid == nid) {
                                imports[i] = importTable;
                                break;
                        }
        }
}

NID_CACHE* nidCache_getHeader(){
        return (NID_CACHE*)libkernel_nid_cache_header;
}
SceNID* nidCache_getCache(){
        return (SceNID*)libkernel_nid_cache;
}
