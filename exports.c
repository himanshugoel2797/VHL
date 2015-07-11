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

#include <psp2/kernel/sysmem.h>
#include "exports.h"

static const UVL_Context *ctx;

static int export_printf(const char* fmt, ...)
{
  #ifndef NO_CONSOLE_OUT
        char buffer[INTERNAL_PRINTF_MAX_LENGTH * 5];  //Larger buffer for exported stuff
        va_list va;
        va_start(va, fmt);
        mini_vsnprintf(buffer, INTERNAL_PRINTF_MAX_LENGTH * 5, fmt, va);
        va_end(va);
        ctx->logline(buffer);
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

int exports_initialize(const UVL_Context *_ctx)
{
        _ctx->psvUnlockMem();
        ctx = _ctx;
        _ctx->psvLockMem();

        nid_table_exportFunc(ctx, &allocCodeMem, NID_ALLOC_CODE_MEM);
        nid_table_exportFunc(ctx, &export_printf, NID_printf);
        nid_table_exportFunc(ctx, ctx->logline, NID_puts);
        nid_table_exportFunc(ctx, ctx->logline, NID_LOG);
        nid_table_exportFunc(ctx, ctx->psvUnlockMem, NID_UNLOCK);
        nid_table_exportFunc(ctx, ctx->psvLockMem, NID_LOCK);
        nid_table_exportFunc(ctx, ctx->psvFlushIcache, NID_FLUSH);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return sceKernelFindMemBlockByAddr(ctx->psvCodeAllocMem(&size), 0);
}

//TODO export module functions so they can be called from other threads
