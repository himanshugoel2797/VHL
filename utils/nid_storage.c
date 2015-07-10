/*
   nid_storage.c : Stores resolved NIDs and hooks with efficient retrieval
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
#include "nid_storage.h"

static nidTable_entry nid_storage_table[NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES];
static nidTable_entry nid_storage_table_hooks[NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER];

int nid_storage_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        for(int i = 0; i < NID_STORAGE_BUCKET_COUNT; i++)
        {
                nid_storage_table[i * NID_STORAGE_MAX_BUCKET_ENTRIES].nid = 0;
                nid_storage_table_hooks[i * NID_STORAGE_HOOK_MULTIPLIER * NID_STORAGE_MAX_BUCKET_ENTRIES].nid = 0;
        }
        calls->LockMem();
        return 0;
}

__attribute__((hot))
int nid_storage_addEntry(VHLCalls *calls, nidTable_entry *entry)
{
        int key = (char)(entry->nid >> 24);
        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES); i < NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                if(nid_storage_table[i].nid == 0 || nid_storage_table[i].nid == entry->nid) { //Search for empty spot to add entry or update duplicate

                        //DEBUG_LOG("Entry %d", i);
                        calls->UnlockMem();
                        //Make sure that the next entry is only cleared if we aren't overwriting an existing entry
                        if(nid_storage_table[i].nid != entry->nid && i + 1 < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES) nid_storage_table[i + 1].nid = 0;

                        nid_storage_table[i].nid = entry->nid;
                        nid_storage_table[i].stub_loc = entry->stub_loc;
                        nid_storage_table[i].type = entry->type;
                        nid_storage_table[i].value.location = entry->value.location;
                        calls->LockMem();
                        return 0;
                }
        }
        DEBUG_LOG_("Failed to add NID");
        return -1;
}

__attribute__((hot))
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

__attribute__((hot))
int nid_storage_addHookEntry(VHLCalls *calls, nidTable_entry *entry)
{

        int key = (char)(entry->nid >> 24);
        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER); i < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER; i++)
        {
                if(nid_storage_table_hooks[i].nid == 0) {   //Search for empty spot to add entry
                        calls->UnlockMem();
                        nid_storage_table_hooks[i].nid = entry->nid;
                        nid_storage_table_hooks[i].stub_loc = entry->stub_loc;
                        nid_storage_table_hooks[i].type = entry->type;
                        nid_storage_table_hooks[i].value.location = entry->value.location;
                        if(i + 1 < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER) nid_storage_table_hooks[i + 1].nid = 0;
                        calls->LockMem();
                        return 0;
                }
        }
        return -1;
}

__attribute__((hot))
int nid_storage_getHookEntry(SceNID nid, nidTable_entry *entry)
{
        int key = (char)(nid >> 24);
        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER); i < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES * NID_STORAGE_HOOK_MULTIPLIER; i++)
        {
                if(nid_storage_table_hooks[i].nid == nid) {
                        entry->nid = nid_storage_table_hooks[i].nid;
                        entry->stub_loc = nid_storage_table_hooks[i].stub_loc;
                        entry->type = nid_storage_table_hooks[i].type;
                        entry->value.location = nid_storage_table_hooks[i].value.location;
                        return 0;
                }
        }
        return -1;
}
