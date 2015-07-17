#ifndef VHL_FS_HOOKS_H
#define VHL_FS_HOOKS_H
#include <psp2/kernel/clib.h>
#include <string.h>
#include "vhl.h"
#include "utils/utils.h"
#include "config.h"
#include "nids.h"

SceUID hook_sceIoOpen(const char* path, int flags, SceMode m);
int hook_sceIoRemove(const char *file);
int hook_sceIoRename (const char *oldname, const char *newname);
SceUID hook_sceIoDopen(const char *dirname);
int hook_sceIoMkdir(const char *dir, SceMode mode);
int hook_sceIoRmdir(const char *path);
int hook_sceIoGetstat(const char *file, SceIoStat *stat);
int hook_sceIoChstat(const char *file, SceIoStat *stat, int bits);

static inline char* TranslateVFS(char *dest, const char *path)
{
        char *p;
        const char *q;

        p = dest;

        //Only substitute the file system path if the root is present
        if(sceClibStrcmp(path, VFS_APPS_DIR) == 1) {
                q = FS_APPS_DIR;
                path += sizeof(FS_APPS_DIR) - 1;
        } else if(sceClibStrcmp(path, VFS_ROOT) == 1) {
                q = FS_ROOT;
                path += sizeof(FS_ROOT) - 1;
        } else
                goto skip;

        while (*q != 0) {
                *p = *q;
                p++;
                q++;
        }

skip:
        strcpy(p, path);
        return dest;
}

#endif
