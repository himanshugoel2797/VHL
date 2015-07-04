#include "elf_parser.h"

static allocData allocatedBlocks[MAX_BLOCK_COUNT];

int elfParser_check_hdr(Elf32_Ehdr *hdr)
{
        if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
                return -1;
        }
        if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
                return -1;
        }
        if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
                return -1;
        }
        if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
                return -1;
        }
        if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {
                DEBUG_LOG_("Unsupported elf file class");
                return -1;
        }
        if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
                DEBUG_LOG_("Unsupported elf target");
                return -1;
        }
        if(hdr->e_machine != EM_ARM) {
                DEBUG_LOG_("Unsupported elf target");
                return -1;
        }
        if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
                return -1;
        }
        if(hdr->e_type != ET_SCE_EXEC && hdr->e_type != ET_EXEC && hdr->e_type != ET_SCE_RELEXEC) {
                DEBUG_LOG_("Unsupported elf type");
                return -1;
        }
        return 0;
}

int elfParser_load_exec(SceUID fd, Elf32_Ehdr *hdr, void **entryPoint)
{

        return -1;
}

int elfParser_load_sce_exec(SceUID fd, Elf32_Ehdr *hdr, void **entryPoint)
{
        return -1;
}

int elfParser_load_sce_relexec(SceUID fd, Elf32_Ehdr *hdr, void **entryPoint)
{
        return -1;
}

int elfParser_Load(VHLCalls *calls, const char *file, void **entryPoint)
{
        char buffer[TEMP_STORAGE_BUFFER_LEN];

        SceUID fd = calls->sceIOOpen(file, PSP2_O_RDONLY, 0777);
        DEBUG_LOG("Opened %s as %d", file, fd);

        unsigned int len = calls->sceIOLseek(fd, 0LL, PSP2_SEEK_END);
        calls->sceIOLseek(fd, 0LL, PSP2_SEEK_SET);
        DEBUG_LOG("File length : %d", len);



        Elf32_Ehdr hdr;
        calls->sceIORead(fd, &hdr, sizeof(Elf32_Ehdr));
        if(elfParser_check_hdr(&hdr) < 0) {
                DEBUG_LOG_("Load failed. Invalid header!");
                return -1;
        }
        switch(hdr.e_type)
        {
        case ET_SCE_RELEXEC:
                return elfParser_load_sce_relexec(fd, &hdr, entryPoint);
                break;
        case ET_SCE_EXEC:
                return elfParser_load_sce_exec(fd, &hdr, entryPoint);
                break;
        case ET_EXEC:
                return elfParser_load_sce_exec(fd, &hdr, entryPoint);
                break;
        default:
                return -1;
                break;
        }
        //TODO figure out how to determine if a homebrew is still running, it might be necessary to export a function to kill a homebrew, along with a hook somewhere in the homebrew to check the status

        return 0;
}
