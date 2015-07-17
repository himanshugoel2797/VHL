/*
   loader.c : Wraps calls to load and launch elfs
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

#include "loader.h"
#include "fs_hooks.h"
#include "state_machine.h"

int hook_sceAppMgrLoadExec(const char *path)
{
        allocData *data = getGlobals()->allocatedBlocks;
        //Trigger a cleanup here

        char tmp[MAX_PATH_LENGTH];
        char *p = TranslateVFS(tmp, path);
        int retVal = elf_parser_load(data, p, NULL);
        retVal = elf_parser_start(data, -1);

        //If we do end up returning, trigger the exitHomebrew procedures
        loader_exitHomebrew(retVal);

        return 0;
        //TODO implement this properly, loads homebrew, kills current homebrew, launches loaded homebrew, maybe this should be moved to state machine
}

int loader_exitHomebrew(int errorCode)
{
        allocData *data = getGlobals()->allocatedBlocks;
        //Trigger a cleanup here

        //Load the menu
        char tmp[MAX_PATH_LENGTH];
        int retVal = elf_parser_load(data, TranslateVFS(tmp, MENU_PATH), NULL);
        if (retVal == 0)
                elf_parser_start(data, -1);

        return errorCode;

}
