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
#ifndef  VHL_VHL_H
#define  VHL_VHL_H

#include <psp2/types.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>

#include "utils/nid_storage.h"
#include "module_headers.h"
#include "common.h"
#include "config.h"
#include "elf_parser.h"

typedef struct {
        int intOptions[INT_VARIABLE_OPTION_COUNT];
        allocData allocatedBlocks[MAX_SLOTS];
        nidTable_entry nid_storage_table[NID_STORAGE_BUCKET_COUNT * NID_STORAGE_MAX_BUCKET_ENTRIES];
} globals_t;

typedef struct {
        void* (*psvCodeAllocMem)(unsigned int *p_len); ///< Allocate code block
        void (*psvUnlockMem)(void);                ///< Unlock code block
        void (*psvLockMem)(void);                  ///< Relock code block
        void (*psvFlushIcache)(void *, unsigned int); ///< Flush Icache
        int (*logline)(const char *);              ///< Debug logging (optional)
        const void *libkernel_anchor;                    ///< Any imported SceLibKernel function
} UVL_Context;

int __attribute__ ((section (".text.start"))) _start(UVL_Context *ctx);
globals_t *getGlobals();

#endif
