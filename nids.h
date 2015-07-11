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
#ifndef _VHL_EXPORT_NIDS_H_
#define _VHL_EXPORT_NIDS_H_

// SceLibKernel
#define NID_sceKernelExitProcess 0x7595D9AA
#define NID_sceIoMkdir 0x9670d39f
#define NID_sceIoOpen 0x6c60ac61
#define NID_sceIoClose 0xc70b8886
#define NID_sceIoRemove 0xe20ed0f3
#define NID_sceIoRename 0xf737e369
#define NID_sceKernelCreateThread 0xc5c11ee7
#define NID_sceKernelStartThread 0xf08de149
#define NID_sceKernelGetThreadId 0x0fb972f9
#define NID_sceKernelGetThreadInfo 0x8d9c5461
#define NID_sceKernelWaitThreadEnd 0xddb395a9
#define NID_sceKernelGetThreadExitStatus 0xd5dc26c4

// SceCtrl
#define NID_sceCtrlPeekBufferPositive 0xa9c3ced6

// SceDisplay
#define NID_sceDisplayWaitVblankStart 0x5795e898

// SceIofilemgr
#define NID_sceIoRead 0xfdb32293
#define NID_sceIoWrite 0x34efd876
#define NID_sceIoLseek 0x99ba173e

// SceModulemgr
#define NID_sceKernelGetModuleList 0x2ef2581f
#define NID_sceKernelGetModuleInfo 0x36585daf
#define NID_sceKernelLoadModule 0xbbe82155
#define NID_sceKernelUnloadModule 0x1987920e
#define NID_sceKernelStopUnloadModule 0x2415f8a4
#define NID_sceKernelLoadStartModule 0x2dcc4afa

// SceSysmem
#define NID_sceKernelAllocMemBlock 0xb9d5ebde
#define NID_sceKernelGetMemBlockBase 0xb8ef5818
#define NID_sceKernelFindMemBlockByAddr 0xa33b99d1
#define NID_sceKernelFreeMemBlock 0xa91e15ee

// SceThreadmgr
#define NID_sceKernelDelayThread 0x4b675d05
#define NID_sceKernelDeleteThread 0x1bbde3d9
#define NID_sceKernelExitDeleteThread 0x1d17decf

// SceLibc
#define NID_printf 0x9A004680
#define NID_puts 0x59C3E171

#define NID_ALLOC_CODE_MEM 0xBCEAB831
#define NID_UNLOCK 0x98D1C91D
#define NID_LOCK 0xEEC99826
#define NID_FLUSH 0xC85B400C
#define NID_LOG 0xD4F59028
#define NID_LOAD_EXEC 3866577596

// VHL
#define NID_vhlGetIntValue 3
#define NID_vhlSetIntValue 4

#endif
