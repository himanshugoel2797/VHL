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
#ifndef _VHL_NIDCACHE_H_
#define _VHL_NIDCACHE_H_

#include "vhl.h"

typedef struct {
        SceNID module_nid;
        SceUInt count;
} NID_CACHE;

int nidCacheContainsModuleNID(SceNID nid, int *offset);

NID_CACHE* nidCache_getHeader();
SceNID* nidCache_getCache();

#endif
