/*
   threadmgr.c : Wraps calls to handle threads
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

#include <psp2/kernel/error.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/types.h>
#include <string.h>
#include <hook/threadmgr.h>
#include <vhl.h>

void initThreadmgr()
{
        globals_t *globals = getGlobals();

        globals->threadmgrSema = sceKernelCreateSema("vhlThreadmgrSema", 0,
                                        MAX_THREADS_NUM, MAX_THREADS_NUM, NULL);
        globals->threadmgrMutex = sceKernelCreateMutex("vhlThreadmgrMutex", 0, 0, NULL);
}

static int exitDeleteCb()
{
        return hook_sceKernelExitDeleteThread(0);
}

static int hookEntry(SceSize args, void *argp)
{
        SceKernelSemaInfo info;
        globals_t *globals;
        SceUID uid;
        int res;

        uid = sceKernelGetThreadId();
        globals = getGlobals();

        sceKernelLockMutexCB(globals->threadmgrMutex, 1, NULL);

        info.size = sizeof(info);
        res = sceKernelGetSemaInfo(globals->threadmgrSema, &info);
        if (res)
                goto fail;

        while (globals->threadmgrTable[info.currentCount].uid != uid) {
                if (info.currentCount >= info.maxCount) {
                        res = SCE_KERNEL_ERROR_ERROR;
                        goto fail;
                }

                info.currentCount++;
        }

        globals->threadmgrTable[info.currentCount].exitDeleteCb
                = sceKernelCreateCallback("vhlUserExitDeleteCb", 0, exitDeleteCb, NULL);

        sceKernelUnlockMutex(globals->threadmgrMutex, 1);

        return globals->threadmgrTable[info.currentCount].entry(args, argp);

fail:
        sceKernelUnlockMutex(globals->threadmgrMutex, 1);
        return res;
}

SceUID hook_sceKernelCreateThread(const char *name, SceKernelThreadEntry entry,
        int initPriority, SceSize stackSize, SceUInt attr,
        int cpuAffinityMask, const SceKernelThreadOptParam *option)
{
        SceKernelSemaInfo info;
        globals_t *globals;
        SceUID uid;
        int res;

        globals = getGlobals();
        sceKernelLockMutexCB(globals->threadmgrMutex, 1, NULL);

        res = sceKernelPollSema(globals->threadmgrSema, 1);
        if (res)
                goto force;

        info.size = sizeof(info);
        res = sceKernelGetSemaInfo(globals->threadmgrSema, &info);
        if (res)
                goto force;

        uid = sceKernelCreateThread(name, hookEntry, initPriority,
                                    stackSize, attr, cpuAffinityMask, option);
        if (uid >= 0) {
                globals->threadmgrTable[info.currentCount].uid = uid;
                globals->threadmgrTable[info.currentCount].entry = entry;
                globals->threadmgrTable[info.currentCount].exitDeleteCb = SCE_KERNEL_ERROR_ERROR;
        }

        sceKernelUnlockMutex(globals->threadmgrMutex, 1);

        return uid;

force:
        sceKernelUnlockMutex(globals->threadmgrMutex, 1);
        return sceKernelCreateThread(name, entry, initPriority,
                                     stackSize, attr, cpuAffinityMask, option);
}

int hook_sceKernelExitDeleteThread(int res)
{
        SceKernelSemaInfo info;
        globals_t *globals;
        SceUID uid;
        int i;

        globals = getGlobals();
        uid = sceKernelGetThreadId();

        DEBUG_PRINTF("Exiting thread 0x%08X (res: 0x%08X)", uid, res);

        sceKernelLockMutex(globals->threadmgrMutex, 1, NULL);

        info.size = sizeof(info);
        i = sceKernelGetSemaInfo(globals->threadmgrSema, &info);
        if (i == 0) {
                for (i = info.currentCount; i < info.maxCount; i++) {
                        if (globals->threadmgrTable[info.currentCount].uid == uid) {
                                if (i != info.currentCount)
                                       memcpy(globals->threadmgrTable + i,
                                              globals->threadmgrTable + info.currentCount,
                                              sizeof(thInfo_t));

                                sceKernelSignalSema(globals->threadmgrSema, 1);
                                break;
                        }
                }
        }

        sceKernelUnlockMutex(globals->threadmgrMutex, 1);
        sceKernelDelayThread(16384);
        return sceKernelExitDeleteThread(res);
}

int terminateDeleteAllUserThreads()
{
        SceKernelSemaInfo semaInfo;
        SceKernelThreadInfo threadInfo;
        globals_t *globals;
        int res;

        globals = getGlobals();
        sceKernelLockMutex(globals->threadmgrMutex, 1, NULL);

        semaInfo.size = sizeof(semaInfo);
        res = sceKernelGetSemaInfo(globals->threadmgrSema, &semaInfo);
        if (res)
                return res;

        while (semaInfo.currentCount < semaInfo.maxCount) {
                res = sceKernelGetThreadInfo(globals->threadmgrTable[semaInfo.currentCount].uid, &threadInfo);
                if (res == 0 && (threadInfo.status == PSP2_THREAD_STOPPED
                                  || threadInfo.status == PSP2_THREAD_KILLED))
                        sceKernelDeleteThread(globals->threadmgrTable[semaInfo.currentCount].uid);
                else
                        sceKernelNotifyCallback(globals->threadmgrTable[semaInfo.currentCount].exitDeleteCb, 0);

                semaInfo.currentCount++;
        }

        sceKernelUnlockMutex(globals->threadmgrMutex, 1);

        sceKernelWaitSema(globals->threadmgrSema, MAX_THREADS_NUM, NULL);
        sceKernelSignalSema(globals->threadmgrSema, MAX_THREADS_NUM);

        return 0;
}

int waitAllUserThreadsEndCB(SceUInt *timeout)
{
        SceUID uid = getGlobals()->threadmgrSema;
        int res;

        res = sceKernelWaitSemaCB(uid, MAX_THREADS_NUM, timeout);
        if (res)
                return res;

        return sceKernelSignalSema(uid, MAX_THREADS_NUM);
}
