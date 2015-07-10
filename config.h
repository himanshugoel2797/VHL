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
#ifndef _VHL_CONFIG_H_
#define _VHL_CONFIG_H_

typedef enum{
  VARIABLE_EXIT_MASK = 1,
  VARIABLE_SUSPEND_MASK = 2
} INT_VARIABLE_OPTIONS;
#define INT_VARIABLE_OPTION_COUNT 3


#define KERNEL_MODULE_SIZE 0x10000

#define MAX_PATH_LENGTH 512

//Homebrew filesystem root
#define FS_ROOT "pss0:/top/Documents/"
#define VFS_ROOT "vfs0:"
//Application directory
#define APPS_PATH FS_ROOT"/app/"
#define VFS_APPS_DIR VFS_ROOT"app/"

#define NID_STORAGE_MAX_BUCKET_ENTRIES 64
#define MAX_SLOTS 64

int config_initialize(void *i);
int config_getIntValue(INT_VARIABLE_OPTIONS option);
int config_setIntValue(INT_VARIABLE_OPTIONS option, int value);

#endif
