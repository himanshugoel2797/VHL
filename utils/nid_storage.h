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
#ifndef _VHL_NID_STORAGE_H_
#define _VHL_NID_STORAGE_H_

#include <psp2/types.h>
#include "../vhl.h"
#include "../config.h"

#define NID_STORAGE_BUCKET_COUNT 256
#define NID_STORAGE_HOOK_MULTIPLIER 1
#define NID_STORAGE_CACHE_FILE VHL_DATA_PATH"/nidCache.bin"


typedef enum  {
        ENTRY_TYPES_FUNCTION,
        ENTRY_TYPES_SYSCALL,
        ENTRY_TYPES_VARIABLE
}EntryTypes;

//Represents an entry in the NID table
typedef struct {
        SceNID nid;
        int type;
        union {
                void *p;
                SceUInt i;
        } value;
} nidTable_entry;

int nid_storage_initialize(const UVL_Context *ctx);
int nid_storage_addEntry(const UVL_Context *ctx, nidTable_entry *entry);
int nid_storage_getEntry(SceNID nid, nidTable_entry *entry);
int nid_storage_addHookEntry(const UVL_Context *ctx, nidTable_entry *entry);
int nid_storage_getHookEntry(SceNID nid, nidTable_entry *entry);

#endif
