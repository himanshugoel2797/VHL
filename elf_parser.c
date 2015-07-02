#include "elf_parser.h"

int elfParser_Load(VHLCalls *calls, const char *file, void **entryPoint)
{
        SceUID fd = calls->sceIOOpen(file, PSP2_O_RDWR, 0);
        DEBUG_LOG("Opened %s as %d", file, fd);

        return 0;
}
