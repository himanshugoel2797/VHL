/*
   mini-printf.c : printf implementation
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

#include <psp2/kernel/clib.h>
#include <stdio.h>
#include <stdarg.h>
#include "mini-printf.h"

int internal_printf(const char *fmt, ...)
{
        char s[512];
        va_list va;

        va_start(va, fmt);
        sceClibVsnprintf(s, 512, fmt, va);
        va_end(va);

        puts(s);

        return 0;
}
