/*
VHL: Vita Homebrew Loader
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
#ifndef VHL_NIDCACHE_H
#define VHL_NIDCACHE_H

#include <module_headers.h>
#include <vhl.h>

enum {
        CACHED_IMPORTED_MODULE_SceSysmem,
        CACHED_IMPORTED_MODULE_SceThreadmgr,
        CACHED_IMPORTED_MODULE_SceModulemgr,
        CACHED_IMPORTED_MODULE_SceProcessmgr,
        CACHED_IMPORTED_MODULE_SceIofilemgr,

        CACHED_IMPORTED_MODULE_NUM
};

typedef struct {
        SceNID module_nid;
        SceUInt count;
} NID_CACHE;

void nidCacheFindCachedImports(const SceModuleInfo *libkernel,
                               const SceModuleImports *imports[CACHED_IMPORTED_MODULE_NUM]);

NID_CACHE* nidCache_getHeader();
SceNID* nidCache_getCache();

#endif
