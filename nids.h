#ifndef _VHL_EXPORT_NIDS_H_
#define _VHL_EXPORT_NIDS_H_

typedef enum {
        ALLOC_CODE_MEM = 0xBCEAB831,
        PRINTF = 0x9A004680,
        PUTS = 1506009457,
        UNLOCK = 0x98D1C91D,
        LOCK = 0xEEC99826,
        FLUSH = 0xC85B400C,
        LOG = 0xD4F59028,
        HOMEBREW_LOAD_NID = 1,
        HOMEBREW_START_NID = 2,
        HOMEBREW_EXIT = 1972754858,
        LOAD_EXEC = 3866577596,
        GET_INT_VALUE = 3,
        SET_INT_VALUE = 4,

} EXPORT_NIDS;

#endif
