#include <stdio.h>
#include "s_inst.h"
#include "uart.h"
#include "mfprintf.h"
#include "exception.h"
#include "csr.h"

// S-type指令处理函数
void execute_s_type_instruction(CPU *cpu, uint32_t instruction) {
    // 提取指令中的各个字段
    uint32_t funct3 = (instruction >> 12) & 0x7; // 功能码
    uint32_t rs1 = (instruction >> 15) & 0x1F;   // 源寄存器1
    uint32_t rs2 = (instruction >> 20) & 0x1F;   // 源寄存器2
    int32_t imm = (int32_t)(((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F)) << 20 >> 20; // 符号扩展立即数


    // 符号扩展立即数
    imm = (imm << 20) >> 20;
    // 计算目标地址
    uint64_t addr = cpu->registers[rs1] + imm;
    if (addr < 0x100 || addr >= MEMORY_SIZE) {
        raise_exception(cpu, CAUSE_LOAD_ACCESS_FAULT);
        cpu->trap_occurred = true;
        return;
    }

    uint32_t size = 0;

    switch (funct3) {
        // SB - 存储字节
        // 示例: sb x2, 0(x1)
        // 将 x2 寄存器的最低字节存储到 x1 寄存器地址加立即数偏移的内存中
        case FUNCT3_SB:
            size = 1;
            break;
        // SH - 存储半字
        // 示例: sh x2, 4(x1)
        // 将 x2 寄存器的最低 2 字节存储到 x1 寄存器地址加立即数偏移的内存中
        case FUNCT3_SH:
            size = 2;
            break;
        // SW - 存储字
        // 示例: sw x2, 8(x1)
        // 将 x2 寄存器的值存储到 x1 寄存器地址加立即数偏移的内存中
        case FUNCT3_SW:
            size = 4;
            break;
        // SD - 存储双字（64位系统下使用）
        // 示例: sd x2, 16(x1)
        // 将 x2 寄存器的值存储到 x1 寄存器地址加立即数偏移的内存中
        case FUNCT3_SD:
            size = 8;
            break;
        default:
            raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }

    memory_write(cpu->memory, addr, cpu->registers[rs2], size);

}
