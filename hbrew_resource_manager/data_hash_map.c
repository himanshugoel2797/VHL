#include "data_hash_map.h"
#include "../utils/bithacks.h"

static int **types[DATA_TYPES_COUNT];

static inline int CalculateKey(int value, int type)
{
        // Type controls the largest space, the most significant byte of the value narrows it down a little, the least significant byte is rounded to a multiple of 3
        return (type << 16) | ((value >> 24) << 8) | ((value << 24) >> 26);
}

static inline int GetValueA(int value)
{
        return (value >> 24) << 8;
}

static inline int GetValueB(int value)
{
        return (value << 24) >> 26;
}

int data_hmap_initialize(VHLCalls *calls)
{
        calls->UnlockMem();
        for(int i = 0; i < DATA_TYPES_COUNT; i++) {
                types[i] = NULL;
        }
        calls->LockMem();
        return 0;
}

__attribute__((hot))
int data_hmap_add(VHLCalls *calls, int value, int type)
{
        int key = CalculateKey(value, type);
        int typeIndex = type;
        int valA = GetValueA(value);
        int valB = GetValueB(value);

        //Allocate memory for the type if it hasn't been allocated
        if(types[typeIndex] == NULL) {
                SceUID uid = calls->sceKernelAllocMemBlock("type", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, FOUR_KB_ALIGN(256 * sizeof(int)), NULL);
                if(uid >= 0) {
                        calls->UnlockMem();
                        calls->sceKernelGetMemBlockBase(uid, &types[typeIndex]);
                        for(int i = 0; i < 256; i++)
                        {
                                types[typeIndex][i] = NULL;
                        }
                        calls->LockMem();
                }else{
                        DEBUG_LOG("Type %d Allocation failed: 0x%08x", type, uid);
                        return -1;
                }
        }

        //Allocate memory for the MSB if it hasn't been allocated
        if(types[typeIndex][valA] == NULL) {
                SceUID uid = calls->sceKernelAllocMemBlock("valA", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, FOUR_KB_ALIGN(MAX_ENTRIES_PER_BUCKET * (256 >> 2) * sizeof(int)), NULL);
                if(uid >= 0) {
                        calls->UnlockMem();
                        calls->sceKernelGetMemBlockBase(uid, &types[typeIndex][valA]);
                        for(int i = 0; i < 256; i++)
                        {
                                types[typeIndex][valA] = NULL;
                        }
                        calls->LockMem();
                }else{
                        DEBUG_LOG("Type %d Value 0%08x Allocation failed: 0x%08x", type, value, uid);
                        return -1;
                }
        }



}
