/*
VHL: Vita Homebrew Loader
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
#ifndef VHL_ELF_PARSER_H
#define VHL_ELF_PARSER_H

#include "config.h"
#include "elf_headers.h"
#include "utils/bithacks.h"

typedef struct {
        void *data_mem_loc;
        SceUID data_mem_uid;
        int data_mem_size;

        void *exec_mem_loc;
        SceUID exec_mem_uid;
        int exec_mem_size;

        void *elf_mem_loc;
        SceUID elf_mem_uid;
        int elf_mem_size;

        char path[MAX_PATH_LENGTH];
        int (*entryPoint)(int, char**);
        SceUID thid;
} allocData;


void block_manager_initialize(void);
int block_manager_free_old_data(allocData *data);

int elf_parser_start(allocData *data, int wait);
int elf_parser_load(allocData *data, const char* file, void** entryPoint);


#endif
