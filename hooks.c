/*
hooks.c : Hooks table
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

#include "common.h"
#include "nids.h"

#include <stdio.h>
#include "config.h"
#include "fs_hooks.h"
#include "loader.h"
#include "state_machine.h"

#define EXPORT(name) { NID_ ## name, name }
#define HOOK(name) { NID_ ## name, hook_ ## name }

static int hook_printf(const char* fmt, ...)
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

typedef struct {
        SceNID nid;
        void *p;
} hook_t;

hook_t forcedHooks[] = {
        HOOK(sceAppMgrLoadExec),
        HOOK(sceIoOpen),
        HOOK(sceIoRemove),
        HOOK(sceIoDopen),
        HOOK(sceIoMkdir),
        HOOK(sceIoRmdir),
        HOOK(sceIoGetstat),
        HOOK(sceIoChstat),
        HOOK(printf),
        { NID_puts, puts },
        EXPORT(vhlGetIntValue),
        EXPORT(vhlSetIntValue)
};
