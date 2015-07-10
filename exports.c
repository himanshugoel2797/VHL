/*
   exports.c : Exports functions for homebrew
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
#include "exports.h"

static VHLCalls func_calls;

static int export_printf(const char* fmt, ...)
{
  #ifndef NO_CONSOLE_OUT
        char buffer[INTERNAL_PRINTF_MAX_LENGTH * 5];  //Larger buffer for exported stuff
        va_list va;
        va_start(va, fmt);
        mini_vsnprintf(buffer, INTERNAL_PRINTF_MAX_LENGTH * 5, fmt, va);
        va_end(va);
        func_calls.LogLine(buffer);
  #else
        va_list va;
        va_start(va, fmt);
        va_end(va);
  #endif
        return 0;
}

static SceUID allocCodeMem(int size)
{
        return AllocCodeMemBlock(size);
}

int exports_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        func_calls.loadAddress = calls->loadAddress;
        func_calls.AllocCodeMem = calls->AllocCodeMem;
        func_calls.FlushICache = calls->FlushICache;
        func_calls.UnlockMem = calls->UnlockMem;
        func_calls.LockMem = calls->LockMem;
        func_calls.LogLine = calls->LogLine;
        func_calls.sceKernelAllocMemBlock = calls->sceKernelAllocMemBlock;
        func_calls.sceKernelFreeMemBlock = calls->sceKernelFreeMemBlock;
        func_calls.sceKernelGetMemBlockBase = calls->sceKernelGetMemBlockBase;
        func_calls.sceKernelFindMemBlockByAddr = calls->sceKernelFindMemBlockByAddr;
        func_calls.sceIOOpen = calls->sceIOOpen;
        func_calls.sceIORead = calls->sceIORead;
        func_calls.sceIOLseek = calls->sceIOLseek;
        func_calls.sceIOClose = calls->sceIOClose;
        calls->LockMem();

        nid_table_exportFunc(calls, &allocCodeMem, ALLOC_CODE_MEM);
        nid_table_exportFunc(calls, &export_printf, PRINTF);
        nid_table_exportFunc(calls, func_calls.LogLine, PUTS);
        nid_table_exportFunc(calls, func_calls.LogLine, LOG);
        nid_table_exportFunc(calls, func_calls.UnlockMem, UNLOCK);
        nid_table_exportFunc(calls, func_calls.LockMem, LOCK);
        nid_table_exportFunc(calls, func_calls.FlushICache, FLUSH);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return func_calls.sceKernelFindMemBlockByAddr(func_calls.AllocCodeMem(&size), 0);
}

//TODO export module functions so they can be called from other threads
