#include "fs_hooks.h"
#include "nid_table.h"

SceUID sceIoOpen_hook(const char* path, int flags, SceMode m)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp = TranslateVFS(tmpPath, path);
        return sceIoOpen(tmp, flags, m);
}

int sceIoRemove_hook(const char *file)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoRemove(tmp);
}

int sceIoRename_hook (const char *oldname, const char *newname)
{

        char o_tmpPath[MAX_PATH_LENGTH];
        char n_tmpPath[MAX_PATH_LENGTH];
        char *o_tmp = TranslateVFS(o_tmpPath, oldname);
        char *n_tmp = TranslateVFS(n_tmpPath, newname);

        return sceIoRename(o_tmp, n_tmp);
}

SceUID sceIoDopen_hook(const char *dirname)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dirname);
        return sceIoDopen(tmp);
}

int sceIoMkdir_hook(const char *dir, SceMode mode)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dir);
        return sceIoMkdir(tmp, mode);
}

int sceIoRmdir_hook(const char *path)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, path);
        return sceIoRmdir(tmp);
}

/*int sceIoChdir_hook(const char *path)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, path);
        return sceIoChdir(tmp);
}*/

int sceIoGetstat_hook(const char *file, SceIoStat *stat)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoGetstat(tmp, stat);
}

int sceIoChstat_hook(const char *file, SceIoStat *stat, int bits)
{
        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoChstat(tmp, stat, bits);
}

int fs_hooks_initialize()
{
        nid_table_registerHook(sceIoOpen_hook, NID_sceIoOpen);
        nid_table_registerHook(sceIoRemove_hook, NID_sceIoRemove);
        nid_table_registerHook(sceIoRename_hook, NID_sceIoRename);
        nid_table_registerHook(sceIoDopen_hook, NID_sceIoDopen);
        nid_table_registerHook(sceIoMkdir_hook, NID_sceIoMkdir);
        nid_table_registerHook(sceIoRmdir_hook, NID_sceIoRmdir);
        //nid_table_registerHook(sceIoChdir_hook, NID_sceIoChdir);
        nid_table_registerHook(sceIoGetstat_hook, NID_sceIoGetstat);
        nid_table_registerHook(sceIoChstat_hook, NID_sceIoChstat);
        return 0;
}
