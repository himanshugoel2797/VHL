#include "elf_parser.h"

int elfParser_Load(VHLCalls *calls, const char *file, void **entryPoint)
{
        SceUID fd = calls->sceIOOpen(file, PSP2_O_RDONLY, 0777);
        DEBUG_LOG("Opened %s as %d", file, fd);

        SceOff len = calls->sceIOLseek(fd, 0, PSP2_SEEK_END);
        calls->sceIOLseek(fd, 0, PSP2_SEEK_SET);
        DEBUG_LOG("Length: %d", len);

        void *elfLoc = calls->AllocCodeMem(((len >> 10) + 1) << 10); //Allocate atleast 1 MB of memory
        calls->sceIORead(fd, elfLoc, len);

        calls->sceIOClose(fd);
        return 0;
}
