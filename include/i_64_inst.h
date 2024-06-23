#ifndef RISCSIMULATOR_I_64_INST_H
#define RISCSIMULATOR_I_64_INST_H
#include "cpu.h"

void execute_i_32_type_instruction(CPU *cpu, uint32_t instruction);
void execute_r_32_type_instruction(CPU *cpu, uint32_t instruction);

#endif //RISCSIMULATOR_I_64_INST_H
