#ifndef _VHL_COMMON_H_
#define _VHL_COMMON_H_

#include <psp2/types.h>

#include "utils/mini-printf.h"

typedef SceUInt SceNID;

//#ifdef DEBUG
        #define DEBUG_LOG(x, ...) internal_printf(x, __VA_ARGS__)
        #define DEBUG_LOG_(x) internal_printf(x)
//#else
//        #define DEBUG_LOG(...)
//        #define DEBUG_LOG_(x)
//#endif


#endif
