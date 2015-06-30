#ifndef _VHL_UTILS_H_
#define _VHL_UTILS_H_

#include <stddef.h>
#include <psp2/types.h>

size_t strlen(const char *str);
char* memstr(char *string, SceUInt len, char *pat, SceUInt patlen);

#endif
