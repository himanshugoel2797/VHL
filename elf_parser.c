/*
   elf_parser.c : parses elf files and loads them
   Copyright (C) 2015  hgoel0974

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */
#include <psp2/kernel/sysmem.h>
#include "utils/utils.h"
#include "elf_parser.h"
#include "nid_table.h"
#include "vhl.h"

int block_manager_free_old_data(allocData *p)
{

        sceKernelFreeMemBlock(p->data_mem_uid);
        sceKernelFreeMemBlock(p->exec_mem_uid);
        sceKernelFreeMemBlock(p->elf_mem_uid);

        p->data_mem_loc = 0;
        p->data_mem_uid = 0;
        p->data_mem_size = 0;
        p->exec_mem_loc = 0;
        p->exec_mem_uid = 0;
        p->exec_mem_size = 0;
        p->elf_mem_loc = 0;
        p->elf_mem_uid = 0;
        p->elf_mem_size = 0;
        p->entryPoint = NULL;
        p->path[0] = 0;

        return 0;
}

void block_manager_initialize()
{
        allocData *allocatedBlocks = getGlobals()->allocatedBlocks;

        for(int curSlot = 0; curSlot < MAX_SLOTS; curSlot++) {
                allocatedBlocks[curSlot].data_mem_loc = 0;
                allocatedBlocks[curSlot].data_mem_uid = 0;
                allocatedBlocks[curSlot].data_mem_size = 0;
                allocatedBlocks[curSlot].exec_mem_loc = 0;
                allocatedBlocks[curSlot].exec_mem_uid = 0;
                allocatedBlocks[curSlot].exec_mem_size = 0;
                allocatedBlocks[curSlot].elf_mem_loc = 0;
                allocatedBlocks[curSlot].elf_mem_uid = 0;
                allocatedBlocks[curSlot].elf_mem_size = 0;
                allocatedBlocks[curSlot].path[0] = 0;
        }
}


/*
   elf_parser_write_segment and elf_parser_relocate taken from UVL
 * relocate.c - Performs SCE ELF relocations
 * Copyright 2015 Yifan Lu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
int elf_parser_write_segment(Elf32_Phdr *phdr, SceUInt offset, void *data, SceUInt len)
{
        if(offset + len > phdr->p_filesz) {
                DEBUG_LOG_("Relocation overflow detected!");
                return -1;
        }
        if(phdr->p_flags & PF_X) {
                sceKernelOpenVMDomain();
        }

        memcpy((char*)phdr->p_vaddr + offset, data, len);

        if(phdr->p_flags & PF_X) {
                sceKernelCloseVMDomain();
        }
        return 0;
}

int elf_parser_relocate(void *reloc, SceUInt size, Elf32_Phdr *segs)
{
        SceReloc *entry;
        SceUInt pos;
        SceUInt16 r_code;
        SceUInt r_offset;
        SceUInt r_addend;
        SceUInt8 r_symseg;
        SceUInt8 r_datseg;
        SceInt offset;
        SceUInt symval, loc;
        SceUInt upper, lower, sign, j1, j2;
        SceUInt value;

        pos = 0;
        while (pos < size)
        {
                // get entry
                entry = (SceReloc *)((char *)reloc + pos);
                if (SCE_RELOC_IS_SHORT (*entry))
                {
                        r_offset = SCE_RELOC_SHORT_OFFSET (entry->r_short);
                        r_addend = SCE_RELOC_SHORT_ADDEND (entry->r_short);
                        pos += 8;
                }
                else
                {
                        r_offset = SCE_RELOC_LONG_OFFSET (entry->r_long);
                        r_addend = SCE_RELOC_LONG_ADDEND (entry->r_long);
                        if (SCE_RELOC_LONG_CODE2 (entry->r_long))
                        {
                                DEBUG_LOG ("Code2 ignored for relocation at %X.", pos);
                        }
                        pos += 12;
                }

                // get values
                r_symseg = SCE_RELOC_SYMSEG (*entry);
                r_datseg = SCE_RELOC_DATSEG (*entry);
                symval = r_symseg == 15 ? 0 : (SceUInt)segs[r_symseg].p_vaddr;
                loc = (SceUInt)segs[r_datseg].p_vaddr + r_offset;

                // perform relocation
                // taken from linux/arch/arm/kernel/module.c of Linux Kernel 4.0
                r_code = SCE_RELOC_CODE(*entry);
                switch (r_code)
                {
                case R_ARM_V4BX:
                {
                        /* Preserve Rm and the condition code. Alter
                         * other bits to re-code instruction as
                         * MOV PC,Rm.
                         */
                        value = (*(SceUInt *)loc & 0xf000000f) | 0x01a0f000;
                }
                break;
                case R_ARM_ABS32:
                case R_ARM_TARGET1:
                {
                        value = r_addend + symval;
                }
                break;
                case R_ARM_REL32:
                case R_ARM_TARGET2:
                {
                        value = r_addend + symval - loc;
                }
                break;
                case R_ARM_THM_CALL:
                {
                        upper = *(SceUInt16 *)loc;
                        lower = *(SceUInt16 *)(loc + 2);

                        /*
                         * 25 bit signed address range (Thumb-2 BL and B.W
                         * instructions):
                         *   S:I1:I2:imm10:imm11:0
                         * where:
                         *   S     = upper[10]   = offset[24]
                         *   I1    = ~(J1 ^ S)   = offset[23]
                         *   I2    = ~(J2 ^ S)   = offset[22]
                         *   imm10 = upper[9:0]  = offset[21:12]
                         *   imm11 = lower[10:0] = offset[11:1]
                         *   J1    = lower[13]
                         *   J2    = lower[11]
                         */
                        sign = (upper >> 10) & 1;
                        j1 = (lower >> 13) & 1;
                        j2 = (lower >> 11) & 1;
                        offset = r_addend + symval - loc;

                        if (offset <= (SceInt)0xff000000 ||
                            offset >= (SceInt)0x01000000) {
                                DEBUG_LOG ("reloc %x out of range: 0x%08X", pos, symval);
                                break;
                        }

                        sign = (offset >> 24) & 1;
                        j1 = sign ^ (~(offset >> 23) & 1);
                        j2 = sign ^ (~(offset >> 22) & 1);
                        upper = (SceUInt16)((upper & 0xf800) | (sign << 10) |
                                            ((offset >> 12) & 0x03ff));
                        lower = (SceUInt16)((lower & 0xd000) |
                                            (j1 << 13) | (j2 << 11) |
                                            ((offset >> 1) & 0x07ff));

                        value = ((SceUInt)lower << 16) | upper;
                }
                break;
                case R_ARM_CALL:
                case R_ARM_JUMP24:
                {
                        offset = r_addend + symval - loc;
                        if (offset <= (SceInt)0xfe000000 ||
                            offset >= (SceInt)0x02000000) {
                                DEBUG_LOG ("reloc %x out of range: 0x%08X", pos, symval);
                                break;
                        }

                        offset >>= 2;
                        offset &= 0x00ffffff;

                        value = (*(SceUInt *)loc & 0xff000000) | offset;
                }
                break;
                case R_ARM_PREL31:
                {
                        offset = r_addend + symval - loc;
                        value = offset & 0x7fffffff;
                }
                break;
                case R_ARM_MOVW_ABS_NC:
                case R_ARM_MOVT_ABS:
                {
                        offset = symval + r_addend;
                        if (SCE_RELOC_CODE (*entry) == R_ARM_MOVT_ABS)
                                offset >>= 16;

                        value = *(SceUInt *)loc;
                        value &= 0xfff0f000;
                        value |= ((offset & 0xf000) << 4) |
                                 (offset & 0x0fff);
                }
                break;
                case R_ARM_THM_MOVW_ABS_NC:
                case R_ARM_THM_MOVT_ABS:
                {
                        upper = *(SceUInt16 *)loc;
                        lower = *(SceUInt16 *)(loc + 2);

                        /*
                         * MOVT/MOVW instructions encoding in Thumb-2:
                         *
                         * i    = upper[10]
                         * imm4 = upper[3:0]
                         * imm3 = lower[14:12]
                         * imm8 = lower[7:0]
                         *
                         * imm16 = imm4:i:imm3:imm8
                         */
                        offset = r_addend + symval;

                        if (SCE_RELOC_CODE (*entry) == R_ARM_THM_MOVT_ABS)
                                offset >>= 16;

                        upper = (SceUInt16)((upper & 0xfbf0) |
                                            ((offset & 0xf000) >> 12) |
                                            ((offset & 0x0800) >> 1));
                        lower = (SceUInt16)((lower & 0x8f00) |
                                            ((offset & 0x0700) << 4) |
                                            (offset & 0x00ff));

                        value = ((SceUInt)lower << 16) | upper;
                }
                break;
                default:
                {
                        DEBUG_LOG ("Unknown relocation code %u at %x", r_code, pos);
                }
                case R_ARM_NONE:
                        continue;
                }

                // write value
                elf_parser_write_segment(&segs[r_datseg], r_offset, &value, sizeof (value));
        }

        return 0;
}

int elf_parser_find_SceModuleInfo(Elf32_Ehdr *elf_hdr, Elf32_Phdr *elf_phdrs, SceModuleInfo **mod_info)
{
        //Src: https://github.com/yifanlu/UVLoader/blob/master/load.c
        SceUInt index = ((SceUInt)elf_hdr->e_entry & 0xC0000000) >> 30;
        SceUInt offset = (SceUInt)elf_hdr->e_entry & 0x3FFFFFFF;

        if ((SceUInt)elf_phdrs[index].p_vaddr == 0)
        {
                DEBUG_LOG ("Invalid segment index %d\n", index);
                return -1;
        }

        *mod_info = (SceModuleInfo*)((char *)elf_phdrs[index].p_vaddr + offset);

        return 0;
}

int elf_parser_check_hdr(Elf32_Ehdr *hdr)
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


int elf_parser_load_exec()
{
        return -1;
}

int elf_parser_load_sce_exec()
{
        return -1;
}

int elf_parser_load_sce_relexec(allocData *data, SceUID fd, unsigned int len, Elf32_Ehdr *hdr, void **entryPoint)
{
        if(data->data_mem_uid != 0) block_manager_free_old_data(data); //Make sure the block is empty to prevent memory leaks
        char tmpDS_name[18];
        snprintf(tmpDS_name, 18, "elf_data_store%08X", data);


        SceUID tmpDataStore_uid = sceKernelAllocMemBlock(tmpDS_name, SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, FOUR_KB_ALIGN(len), NULL);
        void *tmpDataStore_loc = NULL;

        if(tmpDataStore_uid < 0) {
                DEBUG_LOG_("Failed to allocate memory for homebrew");
                return -1;
        }
        if(sceKernelGetMemBlockBase(tmpDataStore_uid, &tmpDataStore_loc) < 0) {
                DEBUG_LOG_("Failed to retrieve allocated memory for homebrew");
                goto freeTmpDataAndError;
        }

        sceIoLseek(fd, 0, PSP2_SEEK_SET);
        if(sceIoRead(fd, tmpDataStore_loc, len) <= 0) {
                DEBUG_LOG_("Read failed");
                goto freeTmpDataAndError;
        }

        //retrieve program sections
        if(hdr->e_phnum < 1) {
                DEBUG_LOG_("No program sections!");
                goto freeTmpDataAndError;
        }

        Elf32_Phdr *prgmHDR = (Elf32_Phdr*)((SceUInt)tmpDataStore_loc + hdr->e_phoff);


        //Start parsing the sections

        //First round is used to calculate the amount of memory of each needed
        int exec_mem_size = 0, data_mem_size = 0;
        SceUID exec_mem_uid = 0, data_mem_uid = 0;
        void *exec_mem_loc = NULL, *data_mem_loc = NULL;

        char name[17];

        for(int i = 0; i < hdr->e_phnum; i++) {
                switch(prgmHDR[i].p_type)
                {
                case PH_LOAD:
                        //Count how much memory to allocate for the Load headers
                        if(prgmHDR[i].p_flags & PF_X) exec_mem_size += prgmHDR[i].p_memsz;
                        else data_mem_size += prgmHDR[i].p_memsz;
                        break;
                default:
                        break;
                }
        }
        //Finally allign this total size to one MB and allocate
        exec_mem_size = MB_ALIGN(FOUR_KB_ALIGN(exec_mem_size));
        data_mem_size = FOUR_KB_ALIGN(data_mem_size);

        snprintf(name, 17, "codeSlot%08X", data);
        exec_mem_uid = sceKernelAllocMemBlockForVM(name, exec_mem_size);
        if(exec_mem_uid < 0) {
                DEBUG_LOG_("Failed to allocate executable memory!");
                goto freeAllAndError;
        }

        if(sceKernelGetMemBlockBase(exec_mem_uid, &exec_mem_loc) < 0) {
                DEBUG_LOG_("Failed to retrieve allocated data memory!");
                goto freeAllAndError;
        }

        snprintf(name, 17, "dataSlot%08X", data);
        data_mem_uid = sceKernelAllocMemBlock(name, SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, data_mem_size, NULL);
        if(data_mem_uid < 0) {
                DEBUG_LOG_("Failed to allocate data memory!");
                goto freeAllAndError;
        }

        if(sceKernelGetMemBlockBase(data_mem_uid, &data_mem_loc) < 0) {
                DEBUG_LOG_("Failed to retrieve allocated data memory!");
                goto freeAllAndError;
        }

        //Update the memory entry table
        data->data_mem_loc = data_mem_loc;
        data->data_mem_uid = data_mem_uid;
        data->data_mem_size = data_mem_size;
        data->exec_mem_loc = exec_mem_loc;
        data->exec_mem_uid = exec_mem_uid;
        data->exec_mem_size = exec_mem_size;
        data->elf_mem_loc = tmpDataStore_loc;
        data->elf_mem_uid = tmpDataStore_uid;
        data->elf_mem_size = FOUR_KB_ALIGN(len);

        //Second round performs the actual parsing and allocation
        void *block_loc = NULL;

        for(int i = 0; i < hdr->e_phnum; i++) {
                switch(prgmHDR[i].p_type)
                {
                case PH_LOAD:
                        DEBUG_LOG_("LOAD header");
                        //Count how much memory to allocate for the Load headers
                        if(prgmHDR[i].p_flags & PF_X)
                        {
                                block_loc = exec_mem_loc;
                                exec_mem_loc += prgmHDR[i].p_memsz;
                                exec_mem_size -= prgmHDR[i].p_memsz;
                        }
                        else
                        {
                                block_loc = data_mem_loc;
                                data_mem_loc += prgmHDR[i].p_memsz;
                                data_mem_size -= prgmHDR[i].p_memsz;
                        }
                        prgmHDR[i].p_vaddr = (SceUInt)block_loc;

                        DEBUG_LOG_("Writing Segment...");
                        elf_parser_write_segment(&prgmHDR[i], 0, (void*)((SceUInt)tmpDataStore_loc + prgmHDR[i].p_offset), prgmHDR[i].p_filesz);

                        sceKernelOpenVMDomain();
                        DEBUG_LOG_("Clearing memory...");
                        memset ((void*)((SceUInt)block_loc + (SceUInt)prgmHDR[i].p_filesz), 0, prgmHDR[i].p_memsz - prgmHDR[i].p_filesz);  //TODO this is failing for some reason
                        sceKernelCloseVMDomain();

                        DEBUG_LOG_("Loaded LOAD section");

                        break;
                case PH_SCE_RELOCATE:
                        DEBUG_LOG_("RELOCATE header");
                        elf_parser_relocate ((void*)((SceUInt)tmpDataStore_loc + prgmHDR[i].p_offset), prgmHDR[i].p_filesz, prgmHDR);
                        break;
                default:
                        DEBUG_LOG("Program Segment %d can not be loaded", i);
                        break;
                }
        }

        //Finally, resolve all stubs
        SceModuleInfo *mod_info;
        int index = elf_parser_find_SceModuleInfo(hdr, prgmHDR, &mod_info);
        if(index < 0)
        {
                DEBUG_LOG_("Failed to find SceModuleInfo section...");
                goto freeAllAndError;
        }
        DEBUG_LOG_("ModuleInfo found");

        FOREACH_IMPORT(prgmHDR[index].p_vaddr, mod_info, imports)
        {
                void **entryTable = GET_FUNCTIONS_ENTRYTABLE(imports);
                SceUInt *nidTable = GET_FUNCTIONS_NIDTABLE(imports);

                for(unsigned int i = 0; i < GET_FUNCTION_COUNT(imports); i++)
                {
                        int err = nid_table_resolveStub(entryTable[i], nidTable[i]);
                        if(err < 0) DEBUG_LOG("Failed to resolve import NID 0x%08x", nidTable[i]);
                }

                entryTable = GET_VARIABLE_ENTRYTABLE(imports);
                nidTable = GET_VARIABLE_NIDTABLE(imports);

                for(int i = 0; i < GET_VARIABLE_COUNT(imports); i++)
                {
                        int err = nid_table_resolveStub(entryTable[i], nidTable[i]);
                        if(err < 0) DEBUG_LOG("Failed to resolve variable NID 0x%08x", nidTable[i]);
                }
        }

        DEBUG_LOG_("Retrieving entry point");
        if(entryPoint != NULL) *entryPoint = (void *)(prgmHDR[index].p_vaddr + mod_info->mod_start);
        data->entryPoint = (void *)(prgmHDR[index].p_vaddr + mod_info->mod_start);

        DEBUG_LOG_("Flushing Icache");
        sceKernelSyncVMDomain(data->exec_mem_uid, data->exec_mem_loc, data->exec_mem_size);
        DEBUG_LOG_("Flushed");

        return 0;

freeAllAndError:
        block_manager_free_old_data(data);
freeTmpDataAndError:
        sceKernelFreeMemBlock(tmpDataStore_uid);
        return -1;
}

int elf_parser_load(allocData *data, const char *file, void **entryPoint)
{
        DEBUG_LOG_("elf_parser_Load");
        SceUID fd = sceIoOpen(file, PSP2_O_RDONLY, 0777);
        DEBUG_LOG("Opened %s as %d", file, fd);

        unsigned int len = sceIoLseek(fd, 0LL, PSP2_SEEK_END);
        sceIoLseek(fd, 0LL, PSP2_SEEK_SET);
        DEBUG_LOG("File length : %d", len);

        strcpy(data->path, file);

        Elf32_Ehdr hdr;
        sceIoRead(fd, &hdr, sizeof(Elf32_Ehdr));
        if(elf_parser_check_hdr(&hdr) < 0) {
                DEBUG_LOG_("Invalid header!");
                return -1;
        }
        switch(hdr.e_type)
        {
        case ET_SCE_RELEXEC:
                return elf_parser_load_sce_relexec(data, fd, len, &hdr, entryPoint);
                break;
        case ET_SCE_EXEC:
                internal_printf("ET_SCE_EXEC format not supported at the moment");
                return elf_parser_load_sce_exec(data, fd, len, &hdr, entryPoint);
                break;
        case ET_EXEC:
                internal_printf("ET_EXEC format not supported at the moment");
                return elf_parser_load_sce_exec(data, fd, len, &hdr, entryPoint);
                break;
        default:
                return -1;
                break;
        }
        sceIoClose(fd);
        //TODO figure out how to determine if a homebrew is still running, it might be necessary to export a function to kill a homebrew, along with a hook somewhere in the homebrew to check the status

        return 0;
}

int homebrew_thread_entry(SceSize args __attribute__((unused)), void *argp)
{

        allocData *data = *(allocData **)argp;
        char tmp[512];
        strcpy(tmp, data->path);

        //TODO start managing resources for this
        int retVal = data->entryPoint(0, NULL);
        //TODO stop managing and clear all resources

        sceKernelExitDeleteThread(retVal);
        return retVal;
}

int elf_parser_start(allocData *data, int wait)
{
        SceKernelThreadInfo mainThreadInfo;
        SceUID tid;

        mainThreadInfo.size = sizeof(SceKernelThreadInfo);
        sceKernelGetThreadInfo(sceKernelGetThreadId(), &mainThreadInfo);

        tid = sceKernelCreateThread("homebrew_thread", homebrew_thread_entry, mainThreadInfo.currentPriority, 0x10000, mainThreadInfo.attr, 0, NULL);
        sceKernelStartThread(tid, sizeof(data), &data);

        data->thid = tid;

        int exitStatus = 0;
        SceUInt *delay = wait < 0 ? NULL : (SceUInt *)&wait;

        sceKernelWaitThreadEnd(tid, &exitStatus, delay);

        return exitStatus;
}
