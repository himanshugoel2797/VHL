/*
   state_machine.c : Manages program state
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
#include "state_machine.h"
#include "nid_table.h"
#include "nids.h"
#include <psp2/ctrl.h>
#include <psp2/display.h>

int hook_sceDisplayWaitVblankStart ();

int state_machine_checkState()
{
        //First check state and exit if necessary
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);

        unsigned int exitMask = vhlGetIntValue(VARIABLE_EXIT_MASK);
        if(exitMask != 0 && pad.buttons == exitMask) {
                //Kill the homebrew
                DEBUG_LOG_("Exit Triggered");
                sceKernelExitDeleteThread(0);
                return 0;
        }

        return 0;
}

//TODO hook all object creation functions for garbage management
//TODO decide on desired means of determining homebrew status so appropriate action can be taken
int hook_sceDisplayWaitVblankStart()
{
        //Check state
        state_machine_checkState();

        //Update state if necessary

        return sceDisplayWaitVblankStart();
}
