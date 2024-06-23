#include <stdint.h>
#include <stdio.h>
#include "b_inst.h"
#include "mfprintf.h"
#include "display.h"

void execute_b_type_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t imm = ((instruction >> 31) << 12) |
                  (((instruction >> 7) & 0x1) << 11) |
                  (((instruction >> 25) & 0x3F) << 5) |
                  (((instruction >> 8) & 0xF) << 1);
    imm = (imm << 19) >> 19;  // 符号扩展立即数

    uint32_t funct3 = (instruction >> 12) & 0x07;

    switch (funct3) {
        case FUNCT3_BEQ:
            // BEQ指令：如果rs1等于rs2，则跳转到pc+imm
            if (cpu->registers[rs1] == cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        case FUNCT3_BNE:
            // BNE指令：如果rs1不等于rs2，则跳转到pc+imm
            if (cpu->registers[rs1] != cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        case FUNCT3_BLT:
            // BLT指令：如果rs1小于rs2，则跳转到pc+imm
            if ((int64_t)cpu->registers[rs1] < (int64_t)cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        case FUNCT3_BGE:
            // BGE指令：如果rs1大于等于rs2，则跳转到pc+imm
            if ((int64_t)cpu->registers[rs1] >= (int64_t)cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        case FUNCT3_BLTU:
            // BLTU指令：如果rs1小于rs2，则跳转到pc+imm
            if ((uint64_t)cpu->registers[rs1] < (uint64_t)cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        case FUNCT3_BGEU:
            // BGEU指令：如果rs1大于等于rs2，则跳转到pc+imm
            if ((uint64_t)cpu->registers[rs1] >= (uint64_t)cpu->registers[rs2]) {
                cpu->pc += imm;
                return;  // 跳转后不需要增加PC
            }
            break;
        default:
            mfprintf("Unknown B-type instruction with funct3: 0x%x\n", funct3);
    }
    cpu->pc += 4;  // 如果没有跳转，则PC增加4
}
