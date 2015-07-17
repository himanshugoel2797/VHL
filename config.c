/*
   config.c : Provides loader configuration options
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
#include <config.h>
#include <vhl.h>
#include <nid_table.h>
#include <nids.h>

int config_initialize()
{
        int *intOptions = getGlobals()->intOptions;

        for(int j = 0; j < INT_VARIABLE_OPTION_COUNT; j++) {
                intOptions[j] = 0;
        }

        return 0;
}

int vhlGetIntValue(INT_VARIABLE_OPTIONS option)
{
        return getGlobals()->intOptions[option - 1];
}

int vhlSetIntValue(INT_VARIABLE_OPTIONS option, int val)
{
        getGlobals()->intOptions[option - 1] = val;
        DEBUG_PRINTF("Option %08x Set to %08x ", option, val);
        return val;
}
