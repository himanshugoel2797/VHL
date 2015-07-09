#include "loader.h"

static VHLCalls *vhl;
static int currentHomebrew;

int loader_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        vhl = calls;
        currentHomebrew = 0;
        calls->LockMem();

        nid_table_exportFunc(calls, loader_loadHomebrew, HOMEBREW_LOAD_NID);
        nid_table_exportFunc(calls, loader_startHomebrew, HOMEBREW_START_NID);
        nid_table_exportFunc(calls, loader_loadExec, LOAD_EXEC);

        return 0;
}

int loader_loadExec(const char *path, const char *argv[], void *opt)
{
    return loader_loadHomebrew(path, 1);
}

int loader_loadHomebrew(const char *path, int slot)
{
    return loader_loadHomebrew(path, 1);
}

int loader_loadHomebrew(const char *path, int slot)
{
        int (*load)(VHLCalls*, int, int, const char*, void**) = ((SceUInt)&elf_parser_load + vhl->loadAddress); //Offset is correct, jump still crashes
        return load(vhl, 1, slot, path, NULL);
}

int loader_startHomebrew(int slot)
{
        vhl->UnlockMem();
        currentHomebrew = slot;
        vhl->LockMem();
        return elf_parser_start(vhl, slot, -1); //Wait endlessly until homebrew exit or suspend
}

int loader_exitHomebrew(int errorCode)
{
      return errorCode;
}
