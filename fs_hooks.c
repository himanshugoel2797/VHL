#include "fs_hooks.h"
#include "nid_table.h"

static VHLCalls *calls;
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

int fs_hooks_initialize(VHLCalls *vhl)
{
        vhl->UnlockMem();
        calls = vhl;
        vhl->LockMem();

        nid_table_registerHook(calls, sceIoOpen_hook, NID_sceIoOpen);
        nid_table_registerHook(calls, sceIoRemove_hook, NID_sceIoRemove);
        nid_table_registerHook(calls, sceIoRename_hook, NID_sceIoRename);
        return 0;
}
