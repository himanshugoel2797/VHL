#include "loader.h"

static VHLCalls *vhl;

int loader_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        vhl = calls;
        calls->LockMem();

        nid_table_exportFunc(calls, loader_loadHomebrew, HOMEBREW_LOAD_NID);
        nid_table_exportFunc(calls, loader_startHomebrew, HOMEBREW_START_NID);

        return 0;
}

int loader_loadHomebrew(const char *path, int slot)
{
        int (*load)(VHLCalls*, int, const char*, void**) = ((SceUInt)&elf_parser_load + vhl->loadAddress); //Offset is correct, jump still crashes
        return load(vhl, slot, path, NULL);
}

int loader_startHomebrew(int slot)
{
        return elf_parser_start(vhl, slot);
}
