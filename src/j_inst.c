#include <stdint.h>
#include <stdio.h>
#include "j_inst.h"

void execute_j_type_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    int32_t imm;

    if ((instruction & 0x7F) == OPCODE_JAL) {
        // 处理JAL指令
        imm = ((instruction >> 31) << 20) |
              (((instruction >> 21) & 0x3FF) << 1) |
              (((instruction >> 20) & 0x1) << 11) |
              (((instruction >> 12) & 0xFF) << 12);
        imm = (imm << 11) >> 11;  // 符号扩展立即数
        if (rd != 0) {
            cpu->registers[rd] = cpu->pc + 4;
        }
        cpu->pc += imm;
    } else if ((instruction & 0x7F) == OPCODE_JALR) {
        // 处理JALR指令
        imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
        uint32_t rs1 = (instruction >> 15) & 0x1F;
        if (rd != 0) {
            cpu->registers[rd] = cpu->pc + 4;
        }
        cpu->pc = (cpu->registers[rs1] + imm) & ~1;  // 确保跳转地址是对齐的
    } else {
        printf("Unknown J-type instruction with opcode: 0x%x\n", instruction & 0x7F);
    }
}