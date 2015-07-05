#ifndef _VHL_UTILS_H_
#define _VHL_UTILS_H_

#include <psp2/types.h>

size_t strlen(const char *str);
char* memstr(char *string, SceUInt len, char *pat, SceUInt patlen);
void * memcpy(void * dst, const void * src, size_t len);
void * memset(void * s, char c, size_t n);

#endif
