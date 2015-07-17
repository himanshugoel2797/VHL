/*
   arm_tools.c : Provides methods to disassemble/assemble ARM assembly
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
#include "arm_tools.h"

/* They shouldn't call any external function because 
 they will be executed before resolving VHL stubs. */

int Disassemble(const void *instruction, ARM_INSTRUCTION *instData)
{
        //TODO implement ARM instruction disassembling

        unsigned int inst = *(unsigned int*)instruction;

        //Extract the condition information
        instData->condition = ARM_CONDITION_EXTRACT(inst);

        //Extract instruction type information
        instData->type = ARM_TYPE_EXTRACT(inst);

        switch(instData->type)
        {
        case ARM_SVC_INSTRUCTION:
                instData->instruction = ARM_INST_SVC;
                instData->value[0] = B_EXTRACT(inst, 23, 0); //An SVC instruction only has one value
                instData->argCount = 1;
                break;
        case ARM_MOV_INSTRUCTION:
                instData->instruction = ARM_EXTRA_TYPE_EXTRACT(inst);
                if(instData->instruction == 14 || instData->instruction == 15)
                {
                        instData->type = ARM_MVN_INSTRUCTION;
                        instData->argCount = 0; //TODO parse MVN arguments
                        break;
                }
                if(instData->instruction != ARM_INST_MOVT && instData->instruction != ARM_INST_MOVW) {
                        instData->type = ARM_UNKN_INSTRUCTION;
                        instData->instruction = ARM_INST_UNKNOWN;
                        break;
                }
                instData->value[0] = B_EXTRACT(inst, 15, 12);
                instData->value[1] = B_JOIN(B_EXTRACT(inst, 11, 0), B_EXTRACT(inst, 19, 16), 12);
                instData->argCount = 2;
                break;
        case ARM_BRANCH_INSTRUCTION:
                instData->instruction = B_EXTRACT(inst, 7, 4);  //The instruction type for a branch depends on 7-4
                if(instData->instruction != ARM_INST_BX && instData->instruction != ARM_INST_BLX) {
                        instData->type = ARM_UNKN_INSTRUCTION;
                        instData->instruction = ARM_INST_UNKNOWN;
                        break;
                }
                instData->value[0] = B_EXTRACT(inst, 3, 0);
                instData->argCount = 1;
                break;
        case ARM_ADR_INSTRUCTION:
                instData->instruction = ARM_EXTRA_TYPE_EXTRACT(inst);
                if(instData->instruction == (Instructions)ARM_ADR_INSTRUCTION) instData->instruction = ARM_INST_ADR; //two possible encodings
                if(instData->instruction != (Instructions)ARM_ADR_INSTRUCTION && instData->instruction != ARM_INST_ADR) {
                        instData->type = ARM_UNKN_INSTRUCTION;
                        instData->instruction = ARM_INST_UNKNOWN;
                        break;
                }
                instData->value[0] = B_EXTRACT(inst, 15, 12);
                instData->value[1] = B_EXTRACT(inst, 11, 0);
                instData->argCount = 2;
                break;
        case ARM_UNKN_INSTRUCTION:
        default:
                instData->type = ARM_UNKN_INSTRUCTION;
                instData->instruction = ARM_INST_UNKNOWN;
                instData->argCount = 0;
                return -1;
        }
        return 0;
}

int Assemble(ARM_INSTRUCTION *instData, SceUInt *instruction)
{



        unsigned int tmp = 0;
        tmp |= B_EXTRACT(instData->condition, 3, 0) << 28;
        tmp |= B_EXTRACT(instData->type, 3, 0) << 24;

        //Generate instruction
        switch(instData->type) {
        case ARM_MOV_INSTRUCTION:
                tmp |= B_EXTRACT(instData->instruction, 3, 0) << 20;
                tmp |= B_EXTRACT(instData->value[0], 3, 0) << 12;

                tmp |= B_EXTRACT(instData->value[1], 11, 0);
                tmp |= B_EXTRACT(instData->value[1], 15, 12) << 16;
                break;
        case ARM_SVC_INSTRUCTION:
                tmp |= B_EXTRACT(instData->value[0], 23, 0);
                break;
        case ARM_BRANCH_INSTRUCTION:
                tmp = ((SceUInt)0xE12FFF1 << 4) | instData->value[0];
                break;
        default:
                return -1;
        }

        *instruction = tmp;

        return 0;
}
