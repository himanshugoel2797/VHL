#include "elf_parser.h"

int elfParser_Load(VHLCalls *calls, const char *file, void **entryPoint)
{
        SceUID fd = calls->sceIOOpen(file, PSP2_O_RDONLY, 0777);
        DEBUG_LOG("Opened %s as %d", file, fd);

        SceOff len = calls->sceIOLseek(fd, 0LL, PSP2_SEEK_END);
        DEBUG_LOG("File length : %d", len);

        char mgNum[4];
        mgNum[3] = 0;

        calls->sceIORead(fd, mgNum, 3);
        DEBUG_LOG("%s", mgNum);

        return 0;
}
