@ stub.s: VHL Stubs
@ Copyright (C) 2015  hgoel0974
@
@ This program is free software; you can redistribute it and/or modify
@ it under the terms of the GNU General Public License as published by
@ the Free Software Foundation; either version 3 of the License, or
@ (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program; if not, write to the Free Software Foundation,
@ Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

#include <nids.h>

#define STUB(name) _STUB NID_ ## name, name
.macro _STUB nid, name
        .global \name
        .type   \name, %function
\name:
        mvn   r0, #0
        bx    lr
        nop
        .word \nid
        .size \name, .-\name
.endm

        .section .text
        .align 2

        .global getVhlStubTop
        .type   getVhlStubTop, %function
getVhlStubTop:
        adr r0, vhlStubTop
        bx  lr
        .size   getVhlStubTop, .-getVhlStubTop

vhlStubTop:
        @ puts will be resolved at first with UVL_Context
        STUB(puts)

        @ sceClibVsnprintf will be resolved in the next to puts for DEBUG_PRINTF
        STUB(sceClibVsnprintf)

        STUB(sceClibStrcmp)
        STUB(sceClibSnprintf)
        STUB(sceKernelExitProcess)
        STUB(sceKernelGetModuleList)
        STUB(sceKernelGetModuleInfo)
        STUB(sceKernelLoadModule)
        STUB(sceKernelUnloadModule)
        STUB(sceKernelStopUnloadModule)
        STUB(sceKernelAllocMemBlock)
        STUB(sceKernelGetMemBlockBase)
        STUB(sceIoRead)
        STUB(sceIoWrite)
        STUB(sceKernelFindMemBlockByAddr)
        STUB(sceKernelFreeMemBlock)
        STUB(sceIoClose)
        STUB(sceKernelDeleteThread)
        STUB(sceKernelExitDeleteThread)
        STUB(sceKernelCreateSema)
        STUB(sceKernelCreateMutex)
        STUB(sceKernelLockMutexCB)
        STUB(sceKernelGetSemaInfo)
        STUB(sceKernelLockMutex)
        STUB(sceKernelWaitSema)
        STUB(sceKernelWaitSemaCB)

        .global vhlPrimaryStubSizeSym
vhlPrimaryStubSizeSym = . - (vhlStubTop + 16)

        STUB(sceKernelLoadStartModule)
        STUB(sceIoMkdir)
        STUB(sceIoLseek)
        STUB(sceIoOpen)
        STUB(sceKernelAllocMemBlockForVM)
        STUB(sceKernelSyncVMDomain)
        STUB(sceKernelOpenVMDomain)
        STUB(sceKernelCloseVMDomain)
        STUB(sceKernelCreateThread)
        STUB(sceKernelStartThread)
        STUB(sceKernelGetThreadId)
        STUB(sceKernelDelayThread)
        STUB(sceKernelGetThreadInfo)
        STUB(sceKernelWaitThreadEnd)
        STUB(sceKernelGetThreadExitStatus)
        STUB(sceKernelCreateCallback)
        STUB(sceKernelNotifyCallback)
        STUB(sceKernelCheckCallback)
        STUB(sceKernelUnlockMutex)
        STUB(sceKernelPollSema)
        STUB(sceKernelSignalSema)
        STUB(sceCtrlPeekBufferPositive)
        STUB(sceDisplayWaitVblankStart)
        STUB(sceIoRemove)
        STUB(sceIoRename)
        STUB(sceIoDopen)
        STUB(sceIoRmdir)
        //STUB(sceIoChdir)
        STUB(sceIoGetstat)
        STUB(sceIoChstat)
        STUB(strcpy)

        .global vhlSecondaryStubSizeSym
vhlSecondaryStubSizeSym = . - (vhlStubTop + 16 + vhlPrimaryStubSizeSym)
