#ifndef VHL_FS_HOOKS_H
#define VHL_FS_HOOKS_H
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
        //Only substitute the file system path if the root is present
        if(strcmp(path, VFS_APPS_DIR) == 1){
                strcpy(dest, FS_APPS_DIR);
                strcat(dest, &path[strlen(VFS_APPS_DIR)]);
                return dest;
        }
        else if(strcmp(path, VFS_ROOT) == 1) {
                strcpy(dest, FS_ROOT);
                strcat(dest, &path[strlen(VFS_ROOT)]);
                return dest;
        }
        return (char *)path;
}

#endif
