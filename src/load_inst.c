#include <stdio.h>
#include "load_inst.h"


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

    switch (funct3) {
        case FUNCT3_LB:
            // LB - 加载字节
            // 示例: lb x1, 0(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载一个字节，符号扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (int8_t)cpu->memory->data[address];
            break;

        case FUNCT3_LH:
            // LH - 加载半字
            // 示例: lh x1, 4(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载两个字节，符号扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (int16_t)*(uint16_t *)&cpu->memory->data[address];
            break;

        case FUNCT3_LW:
            // LW - 加载字
            // 示例: lw x1, 8(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载四个字节，符号扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (int32_t)*(uint32_t *)&cpu->memory->data[address];
            break;

        case FUNCT3_LBU:
            // LBU - 加载无符号字节
            // 示例: lbu x1, 0(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载一个字节，零扩展，存储在 x1 寄存器中
            if (address >= UART_BASE_ADDR && address < UART_BASE_ADDR + 8) {
                // 从 UART 寄存器加载字节
                cpu->registers[rd] = uart_read(cpu->uart, address - UART_BASE_ADDR);
            } else {
                // 从内存加载字节
                cpu->registers[rd] = (uint8_t)cpu->memory->data[address];
            }
            break;

        case FUNCT3_LHU:
            // LHU - 加载无符号半字
            // 示例: lhu x1, 4(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载两个字节，零扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (uint16_t)*(uint16_t *)&cpu->memory->data[address];
            break;

        default:
            // 未知指令处理
            printf("Unknown load instruction with funct3: 0x%x\n", funct3);
            break;
    }
}
