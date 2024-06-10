#include <stdio.h>
#include "l_inst.h"
#include "display.h"
#include "mfprintf.h"
#include "exception.h"
#include "csr.h"


// Load指令处理函数
void execute_load_instruction(CPU *cpu, uint32_t instruction) {
    // 提取指令中的各个字段
    uint32_t funct3 = (instruction >> 12) & 0x7; // 功能码
    uint32_t rd = (instruction >> 7) & 0x1F;     // 目标寄存器
    uint32_t rs1 = (instruction >> 15) & 0x1F;   // 源寄存器
    int32_t imm = (int32_t)(instruction >> 20);  // 立即数

    // 符号扩展立即数
    imm = (imm << 20) >> 20;
    // 内存地址
    uint64_t address = cpu->registers[rs1] + imm;

    bool is_signed = true;
    uint32_t size = 0;

    switch (funct3) {
        case FUNCT3_LB:
            size = 1;
            break;
        case FUNCT3_LH:
            size = 2;
            break;
        case FUNCT3_LW:
            size = 4;
            break;
        case FUNCT3_LD:
            size = 8;
            break;
        case FUNCT3_LBU:
            size = 1;
            is_signed = false;
            break;
        case FUNCT3_LHU:
            size = 2;
            is_signed = false;
            break;
        default:
            raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }

    cpu->registers[rd] = memory_read(cpu->memory, address, size, is_signed);
}
