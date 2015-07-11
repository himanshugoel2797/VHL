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
#ifndef _VHL_UTILS_H_
#define _VHL_UTILS_H_

#include <psp2/types.h>

size_t strlen(const char *str);
char* memstr(char *string, SceUInt len, char *pat, SceUInt patlen);
void* memcpy(void * dst, const void * src, size_t len);
void* memset(void * s, int c, size_t n);
int strcpy(char *dst, const char *src);
int substr(char *dst, const char *src, int start, size_t len);
char* strcat(char *dest, const char *src);
int strcmp(const char *a, const char *b);

#endif
