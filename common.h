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
#ifndef VHL_COMMON_H
#define VHL_COMMON_H

#include <psp2/types.h>
#include <stdio.h>

#include "utils/mini-printf.h"

typedef SceUInt SceNID;

//#ifdef DEBUG
        #define DEBUG_LOG(x, ...) internal_printf(x, __VA_ARGS__)
        #define DEBUG_LOG_(x) puts(x)
//#else
//        #define DEBUG_LOG(...)
//        #define DEBUG_LOG_(x)
//#endif


#endif
