/*
   appmgr.c : Wraps calls to load and launch elfs
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

#include <hook/appmgr.h>
#include <hook/iofilemgr.h>
#include <hook/state_machine.h>

int hook_sceAppMgrLoadExec(const char *path)
{
        globals_t *globals = getGlobals();
        int res;

        TranslateVFS(globals->loadExecPath, path);

        res = sceKernelNotifyCallback(globals->loadExecCb, 0);
        if (res)
                return res;

        return hook_sceKernelExitDeleteThread(0);
}

int hook_sceKernelExitProcess(int res)
{
        res = sceKernelNotifyCallback(getGlobals()->exitCb, res);
        if (res)
                return res;

        return hook_sceKernelExitDeleteThread(0);
}
