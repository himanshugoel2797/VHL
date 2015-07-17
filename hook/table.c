/*
table.c : Hooks table
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

#include <common.h>
#include <nids.h>

#include <stdio.h>
#include <config.h>
#include <hook/iofilemgr.h>
#include <hook/appmgr.h>
#include <hook/state_machine.h>

#define EXPORT(name) { NID_ ## name, name }
#define HOOK(name) { NID_ ## name, hook_ ## name }

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
        { NID_printf, internal_printf },
        { NID_puts, puts },
        EXPORT(vhlGetIntValue),
        EXPORT(vhlSetIntValue)
};
