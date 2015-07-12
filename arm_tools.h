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
#ifndef VHL_ARM_TOOLS_H
#define VHL_ARM_TOOLS_H

#include "utils/bithacks.h"
#include "vhl.h"
#include <psp2/types.h>

#define ARM_MAX_ARGS 2
#define ARM_CONDITION_EXTRACT(x) (B_EXTRACT(x, 31, 28))
#define ARM_TYPE_EXTRACT(x) (B_EXTRACT(x, 27, 24))
#define ARM_EXTRA_TYPE_EXTRACT(x) (B_EXTRACT(x, 23, 20))

typedef enum {
        ARM_R0 = 0,
        ARM_R1 = 1,
        ARM_R2 = 2,
        ARM_R3 = 3,
        ARM_R4 = 4,
        ARM_R5 = 5,
        ARM_R6 = 6,
        ARM_R7 = 7,
        ARM_R8 = 8,
        ARM_R9 = 9,
        ARM_R10 = 10,
        ARM_R11 = 11,
        ARM_R12 = 12,
        ARM_R13 = 13,
        ARM_R14 = 14,
        ARM_R15 = 15
}Registers;

typedef enum {
        ARM_CONDITION_ALWAYS = 14,
        ARM_CONDITION_NEVER = 15
} Conditions;

typedef enum {
        ARM_INST_MOVT = 4,
        ARM_INST_ADR = 8,
        ARM_INST_MVN = 14,
        ARM_INST_MOVW = 0,
        ARM_INST_SVC = 15,
        ARM_INST_BX = 1,
        ARM_INST_BLX = 3,
        ARM_INST_UNKNOWN
}Instructions;

typedef enum {
        ARM_MOV_INSTRUCTION = 3,
        ARM_ADR_INSTRUCTION = 4,
        ARM_SVC_INSTRUCTION = 15,
        ARM_MVN_INSTRUCTION = 14,
        ARM_BRANCH_INSTRUCTION = 1,
        ARM_UNKN_INSTRUCTION
}InstructionType;

typedef struct {
        Conditions condition;
        InstructionType type;
        Instructions instruction;
        unsigned int value[ARM_MAX_ARGS];
        unsigned int argCount;
} ARM_INSTRUCTION;

//Disassemble ARM instruction
int Disassemble(const void *instruction, ARM_INSTRUCTION *instData);
int Assemble(ARM_INSTRUCTION *instData, SceUInt *instruction);

#endif
