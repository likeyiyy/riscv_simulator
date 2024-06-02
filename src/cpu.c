#include <stdio.h>
#include "cpu.h"

void cpu_init(CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
}

void cpu_execute(CPU *cpu, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    uint32_t rd = RD(instruction);         // 提取目的寄存器
    uint32_t funct3 = FUNCT3(instruction); // 提取funct3字段
    uint32_t rs1 = RS1(instruction);       // 提取源寄存器1
    uint32_t rs2 = RS2(instruction);       // 提取源寄存器2
    uint32_t funct7 = FUNCT7(instruction); // 提取funct7字段
    uint32_t imm = IMM(instruction);

    // 打印变量值以16进制格式，并对齐
    printf("opcode: 0x%02X | rd: 0x%02X | funct3: 0x%02X | rs1: 0x%02X | rs2: 0x%02X | funct7: 0x%02X | imm: 0x%03X\n",
           opcode, rd, funct3, rs1, rs2, funct7, imm);

    switch (opcode) {
        case OPCODE_R_TYPE: // 处理R型指令
            switch (funct3) {
                case FUNCT3_ADD_SUB:
                    if (funct7 == FUNCT7_ADD) {
                        // ADD指令：rd = rs1 + rs2
                        cpu->registers[rd] = cpu->registers[rs1] + cpu->registers[rs2];
                    } else if (funct7 == FUNCT7_SUB) {
                        // SUB指令：rd = rs1 - rs2
                        cpu->registers[rd] = cpu->registers[rs1] - cpu->registers[rs2];
                    }
                    break;
                case FUNCT3_SLL:
                    // SLL指令：rd = rs1 << (rs2 & 0x3F)
                    cpu->registers[rd] = cpu->registers[rs1] << (cpu->registers[rs2] & 0x3F);
                    break;
                case FUNCT3_SLT:
                    // SLT指令：rd = (rs1 < rs2) ? 1 : 0
                    cpu->registers[rd] = ((int64_t)cpu->registers[rs1] < (int64_t)cpu->registers[rs2]) ? 1 : 0;
                    break;
                case FUNCT3_SLTU:
                    // SLTU指令：rd = (unsigned(rs1) < unsigned(rs2)) ? 1 : 0
                    cpu->registers[rd] = (cpu->registers[rs1] < cpu->registers[rs2]) ? 1 : 0;
                    break;
                case FUNCT3_XOR:
                    // XOR指令：rd = rs1 ^ rs2
                    cpu->registers[rd] = cpu->registers[rs1] ^ cpu->registers[rs2];
                    break;
                case FUNCT3_SRL_SRA:
                    if (funct7 == FUNCT7_SRL) {
                        // SRL指令：rd = rs1 >> (rs2 & 0x3F)
                        cpu->registers[rd] = cpu->registers[rs1] >> (cpu->registers[rs2] & 0x3F);
                    } else if (funct7 == FUNCT7_SRA) {
                        // SRA指令：rd = ((int64_t)rs1) >> (rs2 & 0x3F)
                        cpu->registers[rd] = ((int64_t)cpu->registers[rs1]) >> (cpu->registers[rs2] & 0x3F);
                    }
                    break;
                case FUNCT3_OR:
                    // OR指令：rd = rs1 | rs2
                    cpu->registers[rd] = cpu->registers[rs1] | cpu->registers[rs2];
                    break;
                case FUNCT3_AND:
                    // AND指令：rd = rs1 & rs2
                    cpu->registers[rd] = cpu->registers[rs1] & cpu->registers[rs2];
                    break;
                    // 其他R型指令
                    // ...
            }
            break;
        case OPCODE_I_TYPE:


            switch (funct3) {
                case FUNCT3_ADDI:
                    // ADDI - 加法立即数
                    // 示例: addi x1, x2, 10
                    // 将 x2 寄存器的值与立即数 10 相加，结果存储在 x1 寄存器中
                    cpu->registers[rd] = cpu->registers[rs1] + (int32_t)imm;
                    break;

                case FUNCT3_SLLI:
                    // SLLI - 逻辑左移立即数
                    // 示例: slli x1, x2, 3
                    // 将 x2 寄存器的值逻辑左移 3 位，结果存储在 x1 寄存器中
                    cpu->registers[rd] = cpu->registers[rs1] << (imm & 0x3F);
                    break;

                case FUNCT3_SLTI:
                    // SLTI - 小于比较立即数
                    // 示例: slti x1, x2, 5
                    // 如果 x2 寄存器的值小于立即数 5，x1 寄存器值设为 1，否则为 0
                    cpu->registers[rd] = ((int64_t)cpu->registers[rs1] < (int64_t)imm) ? 1 : 0;
                    break;

                case FUNCT3_SLTIU:
                    // SLTIU - 无符号小于比较立即数
                    // 示例: sltiu x1, x2, 5
                    // 如果 x2 寄存器的无符号值小于立即数 5，x1 寄存器值设为 1，否则为 0
                    cpu->registers[rd] = (cpu->registers[rs1] < (uint64_t)imm) ? 1 : 0;
                    break;

                case FUNCT3_XORI:
                    // XORI - 按位异或立即数
                    // 示例: xori x1, x2, 15
                    // 将 x2 寄存器的值与立即数 15 按位异或操作，结果存储在 x1 寄存器中
                    cpu->registers[rd] = cpu->registers[rs1] ^ (int32_t)imm;
                    break;

                case FUNCT3_SRLI_SRAI:
                    // SRLI 和 SRAI - 右移立即数
                    // SRLI 示例: srli x1, x2, 2
                    // 如果 imm 的高位为 0，执行逻辑右移 (SRLI)
                    // 将 x2 寄存器的值逻辑右移 2 位，结果存储在 x1 寄存器中

                    if ((imm & 0x200) == FUNCT7_SRLI) {
                        cpu->registers[rd] = (uint64_t)cpu->registers[rs1] >> (imm & 0x3F);
                    } else if ((imm & 0x200) == FUNCT7_SRAI) {
                        // SRAI 示例: srai x1, x2, 2
                        // 如果 imm 的高位为 1，执行算术右移 (SRAI)
                        // 将 x2 寄存器的值算术右移 2 位，结果存储在 x1 寄存器中
                        // 打印cpu->registers[rs1]的值
                        printf("cpu->registers[rs1] = %llu\n", cpu->registers[rs1]);
                        cpu->registers[rd] = (int64_t)cpu->registers[rs1] >> (imm & 0x3F);
                    }
                    break;

                case FUNCT3_ORI:
                    // ORI - 按位或立即数
                    // 示例: ori x1, x2, 20
                    // 将 x2 寄存器的值与立即数 20 按位或操作，结果存储在 x1 寄存器中
                    cpu->registers[rd] = cpu->registers[rs1] | (int32_t)imm;
                    break;

                case FUNCT3_ANDI:
                    // ANDI - 按位与立即数
                    // 示例: andi x1, x2, 30
                    // 将 x2 寄存器的值与立即数 30 按位与操作，结果存储在 x1 寄存器中
                    cpu->registers[rd] = cpu->registers[rs1] & (int32_t)imm;
                    break;

                default:
                    // 未知指令处理
                    printf("Unknown I-type instruction with funct3: 0x%x\n", funct3);
                    break;
            }
            break;
    }
    // 打印所有寄存器的值
    printf("--------------------------------------------\n");
    for (int i = 0; i < 32; i++) {
        printf("x%d = %llu\t", i, cpu->registers[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }

}
