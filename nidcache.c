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

#if defined(PSV_3XX)
#include "nidcache3xx.c"
#endif
#include "nidcache.h"

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
