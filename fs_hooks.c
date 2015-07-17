#include <fs_hooks.h>
#include <nid_table.h>
#include <state_machine.h>

SceUID hook_sceIoOpen(const char* path, int flags, SceMode m)
{
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp = TranslateVFS(tmpPath, path);
        return sceIoOpen(tmp, flags, m);
}

int hook_sceIoRemove(const char *file)
{
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoRemove(tmp);
}

int hook_sceIoRename (const char *oldname, const char *newname)
{
        state_machine_checkState();

        char o_tmpPath[MAX_PATH_LENGTH];
        char n_tmpPath[MAX_PATH_LENGTH];
        char *o_tmp = TranslateVFS(o_tmpPath, oldname);
        char *n_tmp = TranslateVFS(n_tmpPath, newname);

        return sceIoRename(o_tmp, n_tmp);
}

SceUID hook_sceIoDopen(const char *dirname)
{
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dirname);
        return sceIoDopen(tmp);
}

int hook_sceIoMkdir(const char *dir, SceMode mode)
{
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, dir);
        return sceIoMkdir(tmp, mode);
}

int hook_sceIoRmdir(const char *path)
{
        state_machine_checkState();

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
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoGetstat(tmp, stat);
}

int hook_sceIoChstat(const char *file, SceIoStat *stat, int bits)
{
        state_machine_checkState();

        char tmpPath[MAX_PATH_LENGTH];
        char *tmp=TranslateVFS(tmpPath, file);
        return sceIoChstat(tmp, stat, bits);
}
