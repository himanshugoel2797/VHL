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
#include <psp2/ctrl.h>

static VHLCalls *vhl;

int sceDisplayWaitVblankStart_hook ();
#define DISPLAY_VBLANK_NID 1469442200
static int (*vblank_wait_start)();

#define CTRL_PEEK_BUFFER_POSITIVE_NID 2848181974
static int (*PeekBufferPositive)(int, SceCtrlData*, int);

int state_machine_initialize(VHLCalls *calls)
{
        int (*vblank_wait)() = NULL;
        nid_table_registerHook(calls, sceDisplayWaitVblankStart_hook, DISPLAY_VBLANK_NID, &vblank_wait);

        int (*peekBuffer)(int, SceCtrlData*, int) = NULL;
        nid_table_resolveImportFromNID(calls, &peekBuffer, CTRL_PEEK_BUFFER_POSITIVE_NID, NULL, NULL);

        calls->UnlockMem();
        vhl = calls;
        vblank_wait_start = vblank_wait;
        PeekBufferPositive = peekBuffer;
        calls->LockMem();

        return 0;
}

//TODO hook all IO functions for filesystem translation
//TODO hook all object creation functions for garbage management
//TODO decide on desired means of determining homebrew status so appropriate action can be taken
int sceDisplayWaitVblankStart_hook()
{
        //First check state and exit if necessary
        SceCtrlData pad;
        PeekBufferPositive(0, &pad, 1);

        int exitMask = config_getIntValue(VARIABLE_EXIT_MASK);
        int suspendMask = config_getIntValue(VARIABLE_SUSPEND_MASK);

        if(suspendMask != 0 && pad.buttons == suspendMask) {
                //suspend the homebrew
                DEBUG_LOG_("Suspend Triggered");
        }
        if(exitMask != 0 && pad.buttons == exitMask) {
                //Kill the homebrew
                DEBUG_LOG_("Exit Triggered");
                vhl->sceKernelExitDeleteThread(0);
                return 0;
        }
        return vblank_wait_start();
}
