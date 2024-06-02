#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "disassemble.h"

// 定义寄存器名称
const char* reg_names[] = {
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

void disassemble(uint32_t instruction, char* buffer, size_t buffer_size) {
    if (buffer == NULL) {
        return; // 如果缓冲区无效，直接返回
    }

    // 提取操作码 (opcode)
    uint32_t opcode = instruction & 0x7F;

    // 提取目标寄存器 (rd)
    uint32_t rd = (instruction >> 7) & 0x1F;

    // 提取功能码 (funct3)
    uint32_t funct3 = (instruction >> 12) & 0x07;

    // 提取源寄存器1 (rs1)
    uint32_t rs1 = (instruction >> 15) & 0x1F;

    // 提取源寄存器2 (rs2)
    uint32_t rs2 = (instruction >> 20) & 0x1F;

    // 提取功能码 (funct7)
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    // 提取立即数 (imm)
    int32_t imm = (instruction >> 20);

    // 提取立即数 (shamt) - 仅低5位有效
    int32_t shamt = (instruction >> 20) & 0x1F;

    // 判断操作码和功能码来反汇编指令
    if (opcode == 0x13) {  // I型指令
        switch (funct3) {
            case 0x0: // ADDI
                snprintf(buffer, buffer_size, "ADDI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            case 0x1: // SLLI
                snprintf(buffer, buffer_size, "SLLI %s, %s, %d", reg_names[rd], reg_names[rs1], imm & 0x1F); // 立即数只有低5位有意义
                break;
            case 0x2: // SLTI
                snprintf(buffer, buffer_size, "SLTI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            case 0x3: // SLTIU
                snprintf(buffer, buffer_size, "SLTIU %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            case 0x4: // XORI
                snprintf(buffer, buffer_size, "XORI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            case 0x5:
                if (funct7 == 0x00) {
                    snprintf(buffer, 100, "SRLI %s, %s, %d", reg_names[rd], reg_names[rs1], shamt);
                } else if (funct7 == 0x20) {
                    snprintf(buffer, 100, "SRAI %s, %s, %d", reg_names[rd], reg_names[rs1], shamt);
                }
                break;
            case 0x6: // ORI
                snprintf(buffer, buffer_size, "ORI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            case 0x7: // ANDI
                snprintf(buffer, buffer_size, "ANDI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                break;
            default:
                snprintf(buffer, buffer_size, "I: 0x%08x", instruction);
        }
    } else if (opcode == 0x33) {  // R型指令
        switch (funct3) {
            case 0x0:
                if (funct7 == 0x00) {
                    snprintf(buffer, buffer_size, "ADD %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                } else if (funct7 == 0x20) {
                    snprintf(buffer, buffer_size, "SUB %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                }
                break;
            case 0x1: // SLL
                snprintf(buffer, buffer_size, "SLL %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            case 0x2: // SLT
                snprintf(buffer, buffer_size, "SLT %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            case 0x3: // SLTU
                snprintf(buffer, buffer_size, "SLTU %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            case 0x4: // XOR
                snprintf(buffer, buffer_size, "XOR %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            case 0x5:
                if (funct7 == 0x00) {
                    snprintf(buffer, buffer_size, "SRL %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                } else if (funct7 == 0x20) {
                    snprintf(buffer, buffer_size, "SRA %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                }
                break;
            case 0x6: // OR
                snprintf(buffer, buffer_size, "OR %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            case 0x7: // AND
                snprintf(buffer, buffer_size, "AND %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                break;
            default:
                snprintf(buffer, buffer_size, "R: 0x%08x", instruction);
        }
    } else {
        snprintf(buffer, buffer_size, "E: 0x%08x", instruction);
    }
}
