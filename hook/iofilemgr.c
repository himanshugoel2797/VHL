/*
   iofilemgr.c : Hooks for file I/O manager
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

#include <psp2/kernel/threadmgr.h>
#include <hook/iofilemgr.h>
#include <nid_table.h>

SceUID hook_sceIoOpen(const char* path, int flags, SceMode m)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp = TranslateVFS(tmpPath, path);
        return sceIoOpen(tmp, flags, m);
}

int hook_sceIoRemove(const char *file)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoRemove(tmp);
}

int hook_sceIoRename (const char *oldname, const char *newname)
{
        sceKernelCheckCallback();

        char o_tmpPath[MAX_PATH_LENGTH];
        char n_tmpPath[MAX_PATH_LENGTH];
        char *o_tmp = TranslateVFS(o_tmpPath, oldname);
        char *n_tmp = TranslateVFS(n_tmpPath, newname);

        return sceIoRename(o_tmp, n_tmp);
}

SceUID hook_sceIoDopen(const char *dirname)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dirname);
        return sceIoDopen(tmp);
}

int hook_sceIoMkdir(const char *dir, SceMode mode)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dir);
        return sceIoMkdir(tmp, mode);
}

int hook_sceIoRmdir(const char *path)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, path);
        return sceIoRmdir(tmp);
}

/*int hook_sceIoChdir(const char *path)
   {
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, path);
        return sceIoChdir(tmp);
   }*/

int hook_sceIoGetstat(const char *file, SceIoStat *stat)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoGetstat(tmp, stat);
}

int hook_sceIoChstat(const char *file, SceIoStat *stat, int bits)
{
        sceKernelCheckCallback();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoChstat(tmp, stat, bits);
}
