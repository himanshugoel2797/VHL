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
#include <psp2/pss.h>
#include <stdio.h>
#include "exports.h"

static int export_printf(const char* fmt, ...)
{
  #ifndef NO_CONSOLE_OUT
        char buffer[INTERNAL_PRINTF_MAX_LENGTH * 5];  //Larger buffer for exported stuff
        va_list va;
        va_start(va, fmt);
        mini_vsnprintf(buffer, INTERNAL_PRINTF_MAX_LENGTH * 5, fmt, va);
        va_end(va);
        puts(buffer);
  #else
        va_list va;
        va_start(va, fmt);
        va_end(va);
  #endif
        return 0;
}

int exports_initialize()
{
        nid_table_exportFunc(pss_code_mem_alloc, NID_ALLOC_CODE_MEM);
        nid_table_exportFunc(export_printf, NID_printf);
        nid_table_exportFunc(puts, NID_puts);
        nid_table_exportFunc(puts, NID_LOG);
        nid_table_exportFunc(pss_code_mem_unlock, NID_UNLOCK);
        nid_table_exportFunc(pss_code_mem_lock, NID_LOCK);
        nid_table_exportFunc(pss_code_mem_flush_icache, NID_FLUSH);

        return 0;
}

SceUID AllocCodeMemBlock(int size)
{
        return sceKernelFindMemBlockByAddr(pss_code_mem_alloc(&size), 0);
}

//TODO export module functions so they can be called from other threads
