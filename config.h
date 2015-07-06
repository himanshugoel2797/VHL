#ifndef _VHL_CONFIG_H_
#define _VHL_CONFIG_H_

#define KERNEL_MODULE_SIZE 0x10000

//Homebrew filesystem root
#define FS_ROOT "pss0:/top/Documents/vfs"

//Application directory
#define APPS_PATH FS_ROOT"/apps/"

//VHL data directory
#define VHL_DATA_PATH FS_ROOT"/vhl/"


#define MAX_SLOTS 32
#define NID_STORAGE_MAX_BUCKET_ENTRIES 64

#endif
