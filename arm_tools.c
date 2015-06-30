#include "arm_tools.h"

int Disassemble(void *instruction, ARM_INSTRUCTION *instData)
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
                        instData->type = ARM_MOV_INSTRUCTION;
                        instData->argCount = 0; //TODO parse MVN arguments
                        break;
                }
                instData->value[0] = B_EXTRACT(inst, 15, 12);
                instData->value[1] = B_JOIN(B_EXTRACT(inst, 11, 0), B_EXTRACT(inst, 19, 16), 12);
                instData->argCount = 2;
                break;
        case ARM_BRANCH_INSTRUCTION:
                instData->instruction = B_EXTRACT(inst, 7, 4);  //The instruction type for a branch depends on 7-4
                instData->value[0] = B_EXTRACT(inst, 3, 0);
                instData->argCount = 1;
                break;
        case ARM_ADR_INSTRUCTION:
                instData->instruction = ARM_EXTRA_TYPE_EXTRACT(inst);
                if(instData->instruction == 4) instData->instruction = ARM_INST_ADR; //two possible encodings
                instData->value[0] = B_EXTRACT(inst, 15, 12);
                instData->value[1] = B_EXTRACT(inst, 11, 0);
                instData->argCount = 2;
                break;
        case ARM_UNKNOWN:
        default:
                instData->type = ARM_UNKNOWN;
                instData->instruction = ARM_INST_UNKNOWN;
                instData->argCount = 0;
                return -1;
        }
        return 0;
}
