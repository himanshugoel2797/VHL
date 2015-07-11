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
#ifndef  _VHL_VHL_H_
#define  _VHL_VHL_H_

#include <psp2/types.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>

#include "module_headers.h"
#include "common.h"

typedef struct {
        SceUInt loadAddress;

        //UVL context calls
        void* (*AllocCodeMem)(SceUInt*);
        void (*UnlockMem)();
        void (*LockMem)();
        void (*FlushICache)(void*, SceUInt);
        int (*LogLine)(const char*);
} VHLCalls;

typedef struct {
        void* (*psvCodeAllocMem)(unsigned int *p_len); ///< Allocate code block
        void (*psvUnlockMem)(void);                ///< Unlock code block
        void (*psvLockMem)(void);                  ///< Relock code block
        void (*psvFlushIcache)(void *, unsigned int); ///< Flush Icache
        int (*logline)(const char *);              ///< Debug logging (optional)
        void *libkernel_anchor;                    ///< Any imported SceLibKernel function
} UVL_Context;

int __attribute__ ((section (".text.start"))) _start(UVL_Context *ctx);
void logLine(const char *str);


#endif
