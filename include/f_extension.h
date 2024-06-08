#ifndef F_EXTENSION_H
#define F_EXTENSION_H

#include "cpu.h"

// F 扩展指令的 funct3 和 funct7
#define FUNCT7_F 0x00

#define FUNCT3_FADD_S  0x0
#define FUNCT3_FSUB_S  0x1
#define FUNCT3_FMUL_S  0x2
#define FUNCT3_FDIV_S  0x3
#define FUNCT3_FSQRT_S 0x4
#define FUNCT3_FSGNJ_S 0x5
#define FUNCT3_FSGNJN_S 0x6
#define FUNCT3_FSGNJX_S 0x7
#define FUNCT3_FMIN_S  0x8
#define FUNCT3_FMAX_S  0x9

void execute_f_extension_instruction(CPU *cpu, uint32_t instruction);


#endif // F_EXTENSION_H
