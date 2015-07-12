#ifndef _VHL_FS_HOOKS_H_
#define _VHL_FS_HOOKS_H_
#include "vhl.h"
#include "utils/utils.h"
#include "config.h"
#include "nids.h"

int fs_hooks_initialize();

static inline char* TranslateVFS(char *dest, char *path)
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
        return path;
}

#endif
