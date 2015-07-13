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

#include "nids.h"

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

        @ The order corresponds to UVL_Context.
vhlStubTop:
        STUB(pss_code_mem_alloc)
        STUB(pss_code_mem_unlock)
        STUB(pss_code_mem_lock)
        STUB(pss_code_mem_flush_icache)
        STUB(puts)

        .global vhlStubCtxSizeSym
vhlStubCtxSizeSym = . - vhlStubTop

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

        .global vhlStubPrimarySizeSym
vhlStubPrimarySizeSym = . - (vhlStubTop + vhlStubCtxSizeSym)

        STUB(sceKernelLoadStartModule)
        STUB(sceIoMkdir)
        STUB(sceIoLseek)
        STUB(sceIoOpen)
        STUB(sceKernelCreateThread)
        STUB(sceKernelStartThread)
        STUB(sceKernelGetThreadId)
        STUB(sceKernelDelayThread)
        STUB(sceKernelGetThreadInfo)
        STUB(sceKernelWaitThreadEnd)
        STUB(sceKernelGetThreadExitStatus)
        STUB(sceCtrlPeekBufferPositive)
        STUB(sceDisplayWaitVblankStart)
        STUB(sceIoRemove)
        STUB(sceIoRename)
        STUB(sceIoDopen)
        STUB(sceIoRmdir)
        //STUB(sceIoChdir)
        STUB(sceIoGetstat)
        STUB(sceIoChstat)

        .global vhlStubSecondarySizeSym
vhlStubSecondarySizeSym = . - (vhlStubTop + vhlStubCtxSizeSym + vhlStubPrimarySizeSym)
