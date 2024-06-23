#ifndef M_EXTENSION_H
#define M_EXTENSION_H

#include "cpu.h"

// M 扩展指令的 funct3 和 funct7
#define FUNCT7_M      0x01
#define FUNCT3_MUL    0x0
#define FUNCT3_MULH   0x1
#define FUNCT3_MULHSU 0x2
#define FUNCT3_MULHU  0x3
#define FUNCT3_DIV    0x4
#define FUNCT3_DIVU   0x5
#define FUNCT3_REM    0x6
#define FUNCT3_REMU   0x7

void execute_m_extension_instruction(CPU *cpu, uint32_t instruction);


#endif // M_EXTENSION_H
