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

#ifndef HOOK_THREADMGR_H
#define HOOK_THREADMGR_H

#include <psp2/kernel/threadmgr.h>
#include <psp2/types.h>

typedef struct {
        SceUID uid;
        SceUID exitDeleteCb;
        SceKernelThreadEntry entry;
} thInfo_t;

void initThreadmgr(void);

SceUID hook_sceKernelCreateThread(const char *name, SceKernelThreadEntry entry,
        int initPriority, SceSize stackSize, SceUInt attr,
        int cpuAffinityMask, const SceKernelThreadOptParam *option);

int hook_sceKernelExitDeleteThread(int res);

int terminateDeleteAllUserThreads(void);
int waitAllUserThreadsEndCB(SceUInt *timeout);

#endif
