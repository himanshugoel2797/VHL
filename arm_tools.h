#ifndef _VHL_ARM_TOOLS_H_
#define _VHL_ARM_TOOLS_H_

#include <psp2/types.h>
#include "bithacks.h"

#define ARM_MAX_ARGS 2

#define ARM_CONDITION_EXTRACT(x) B_EXTRACT(x, 31, 28)
#define ARM_TYPE_EXTRACT(x) B_EXTRACT(x, 27, 24)

#define ARM_EXTRA_TYPE_EXTRACT(x) B_EXTRACT(x, 23, 20)

enum Conditions {
        ARM_CONDITION_ALWAYS = 14,
        ARM_CONDITION_NEVER = 15
};

enum Registers {
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
};

enum Instructions {
        ARM_INST_MOVT = 4,
        ARM_INST_ADR = 8,
        ARM_INST_MVN = 14,
        ARM_INST_MOVW = 0,
        ARM_INST_SVC = 15,
        ARM_INST_BX = 1,
        ARM_INST_BLX = 3,
        ARM_INST_UNKNOWN
};

enum InstructionType {
        ARM_MOV_INSTRUCTION = 3,
        ARM_ADR_INSTRUCTION = 4,
        ARM_SVC_INSTRUCTION = 15,
        ARM_MVN_INSTRUCTION = 14,
        ARM_BRANCH_INSTRUCTION = 1
};

typedef struct {
        unsigned int condition;
        unsigned int type;
        unsigned int instruction;
        unsigned int value[ARM_MAX_ARGS];
        unsigned int argCount;
} ARM_INSTRUCTION;

//Disassemble ARM instruction
int Disassemble(void *instruction, ARM_INSTRUCTION *instData);
int Assemble(ARM_INSTRUCTION instData, unsigned int *instruction);

#endif
