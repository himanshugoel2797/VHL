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
        HOMEBREW_EXIT = 1972754858,
        LOAD_EXEC = 3866577596,
        GET_INT_VALUE = 3,
        SET_INT_VALUE = 4,

} EXPORT_NIDS;

#endif
