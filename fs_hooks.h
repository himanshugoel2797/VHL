#ifndef _VHL_FS_HOOKS_H_
#define _VHL_FS_HOOKS_H_
#include "vhl.h"
#include "utils/utils.h"
#include "config.h"
#include "nids.h"

int fs_hooks_initialize(VHLCalls *calls);

static inline char* TranslateVFS(char *dest, char *path)
{
        //Only substitute the file system path if the root is present
        if(strcmp(path, VFS_ROOT) == 1) {
                DEBUG_LOG_("Translating path");
                char *updatedPath = dest;
                DEBUG_LOG_(path);
                strcpy(updatedPath, FS_ROOT);
                DEBUG_LOG_(updatedPath);
                DEBUG_LOG("%d", strlen(VFS_ROOT));
                strcat(updatedPath, &path[strlen(VFS_ROOT)]);
                DEBUG_LOG_(updatedPath);
                return updatedPath;
        }
        return path;
}

#endif
