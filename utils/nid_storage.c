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
#include "../vhl.h"

int nid_storage_initialize()
{
        nidTable_entry *nid_storage_table = getGlobals()->nid_storage_table;

        for(int i = 0; i < NID_STORAGE_BUCKET_COUNT; i++)
        {
                nid_storage_table[i * NID_STORAGE_MAX_BUCKET_ENTRIES].nid = 0;
        }

        return 0;
}

__attribute__((hot))
int nid_storage_addEntry(nidTable_entry *entry)
{
        nidTable_entry *nid_storage_table = getGlobals()->nid_storage_table;
        int key = (char)(entry->nid >> 24);

        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES); i < NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                if(nid_storage_table[i].nid == 0 || nid_storage_table[i].nid == entry->nid) { //Search for empty spot to add entry or update duplicate

                        //DEBUG_PRINTF("Entry %d", i);
                        //Make sure that the next entry is only cleared if we aren't overwriting an existing entry
                        if(nid_storage_table[i].nid != entry->nid && i + 1 < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES) nid_storage_table[i + 1].nid = 0;

                        nid_storage_table[i].nid = entry->nid;
                        nid_storage_table[i].type = entry->type;
                        nid_storage_table[i].value.i = entry->value.i;

                        return 0;
                }
        }
        DEBUG_PUTS("Failed to add NID");
        return -1;
}

__attribute__((hot))
int nid_storage_getEntry(SceNID nid, nidTable_entry *entry)
{
        nidTable_entry *nid_storage_table = getGlobals()->nid_storage_table;
        int key = (char)(nid >> 24);

        for(int i = (key * NID_STORAGE_MAX_BUCKET_ENTRIES); i < (key + 1) * NID_STORAGE_MAX_BUCKET_ENTRIES; i++)
        {
                if(nid_storage_table[i].nid == nid) {
                        entry->nid = nid_storage_table[i].nid;
                        entry->type = nid_storage_table[i].type;
                        entry->value.i = nid_storage_table[i].value.i;
                        return 0;
                }
        }
        return -1;
}
