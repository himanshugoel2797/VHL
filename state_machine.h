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
#ifndef VHL_STATE_MACHINE_H
#define VHL_STATE_MACHINE_H

#include "vhl.h"
#include "config.h"

#define MAX_THREAD_COUNT 128
#define MAX_OTHER_OBJECT_COUNT 2048 //Max number of Mutex, Semaphore, Event flags, Condition variables, ReadWrite locks, lightweight mutex, lightweight condition variable, events, message pipes, timers, memory blocks

int state_machine_pollControls();
int state_machine_checkState();
int hook_sceDisplayWaitVblankStart(void);

typedef enum{
  WAITING = 0,
  RUNNING = 1,
  EXITING = 2
} HomebrewRunStatus;

typedef struct{
  int currentIndex;
  SceUID objects[MAX_OTHER_OBJECT_COUNT];
} ObjectData;

typedef struct{
  int currentIndex;
  SceUID threads[MAX_THREAD_COUNT];
} ThreadData;

typedef struct{
  HomebrewRunStatus runStatus;
  ObjectData n_mutex;
  ObjectData n_semaphore;
  ObjectData n_eventFlag;
  ObjectData n_condVar;
  ObjectData n_rwLock;
  ObjectData lw_mutex;
  ObjectData lw_condVar;
  ObjectData n_event;
  ObjectData n_msgPipe;
  ObjectData n_timer;
  ObjectData n_memBlock;
  ThreadData n_thread;
} HomebrewState;

#endif
