#include <stdio.h>
#include "cpu.h"
#include "memory.h"
void execute_r_type_instruction(CPU *cpu, uint32_t instruction);
void execute_i_type_instruction(CPU *cpu, uint32_t instruction);
void execute_s_type_instruction(CPU *cpu, Memory *memory, uint32_t instruction);
void execute_load_instruction(CPU *cpu, Memory *memory, uint32_t instruction);

void cpu_init(CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
}


void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码

    switch (opcode) {
        case OPCODE_R_TYPE: // 处理R型指令
            execute_r_type_instruction(cpu, instruction);
            break;
        case OPCODE_I_TYPE:
            break;
        case OPCODE_S_TYPE:
            execute_s_type_instruction(cpu, memory, instruction);
            break;
        case OPCODE_LOAD:
            // 处理Load指令
            execute_load_instruction(cpu, memory, instruction);
            break;
    }
}

// R-type指令处理函数
void execute_r_type_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    uint32_t rd = RD(instruction);         // 提取目的寄存器
    uint32_t funct3 = FUNCT3(instruction); // 提取funct3字段
    uint32_t rs1 = RS1(instruction);       // 提取源寄存器1
    uint32_t rs2 = RS2(instruction);       // 提取源寄存器2
    uint32_t funct7 = FUNCT7(instruction); // 提取funct7字段
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
}

// I-type指令处理函数
void execute_i_type_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    uint32_t rd = RD(instruction);         // 提取目的寄存器
    uint32_t funct3 = FUNCT3(instruction); // 提取funct3字段
    uint32_t rs1 = RS1(instruction);       // 提取源寄存器1
    uint32_t rs2 = RS2(instruction);       // 提取源寄存器2
    uint32_t funct7 = FUNCT7(instruction); // 提取funct7字段
    uint32_t imm = IMM(instruction);
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
}


// S-type指令处理函数
void execute_s_type_instruction(CPU *cpu, Memory *memory, uint32_t instruction) {
    // 提取指令中的各个字段
    uint32_t funct3 = (instruction >> 12) & 0x7; // 功能码
    uint32_t rs1 = (instruction >> 15) & 0x1F;   // 源寄存器1
    uint32_t rs2 = (instruction >> 20) & 0x1F;   // 源寄存器2
    int32_t imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F); // 立即数

    // 符号扩展立即数
    imm = (imm << 20) >> 20;

    switch (funct3) {
        case FUNCT3_SB:
            // SB - 存储字节
            // 示例: sb x2, 0(x1)
            // 将 x2 寄存器的最低字节存储到 x1 寄存器地址加立即数偏移的内存中
            memory->data[cpu->registers[rs1] + imm] = cpu->registers[rs2] & 0xFF;
            break;

        case FUNCT3_SH:
            // SH - 存储半字
            // 示例: sh x2, 4(x1)
            // 将 x2 寄存器的最低 2 字节存储到 x1 寄存器地址加立即数偏移的内存中
            *(uint16_t *)&memory->data[cpu->registers[rs1] + imm] = cpu->registers[rs2] & 0xFFFF;
            break;

        case FUNCT3_SW:
            // SW - 存储字
            // 示例: sw x2, 8(x1)
            // 将 x2 寄存器的值存储到 x1 寄存器地址加立即数偏移的内存中
            *(uint32_t *)&memory->data[cpu->registers[rs1] + imm] = cpu->registers[rs2];
            break;

        case FUNCT3_SD:
            // SD - 存储双字（64位系统下使用）
            // 示例: sd x2, 16(x1)
            // 将 x2 寄存器的值存储到 x1 寄存器地址加立即数偏移的内存中
            *(uint64_t *)&memory->data[cpu->registers[rs1] + imm] = cpu->registers[rs2];
            break;

        default:
            // 未知指令处理
            printf("Unknown S-type instruction with funct3: 0x%x\n", funct3);
            break;
    }
}

// Load指令处理函数
void execute_load_instruction(CPU *cpu, Memory *memory, uint32_t instruction) {
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
            cpu->registers[rd] = (int8_t)memory->data[address];
            break;

        case FUNCT3_LH:
            // LH - 加载半字
            // 示例: lh x1, 4(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载两个字节，符号扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (int16_t)*(uint16_t *)&memory->data[address];
            break;

        case FUNCT3_LW:
            // LW - 加载字
            // 示例: lw x1, 8(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载四个字节，符号扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (int32_t)*(uint32_t *)&memory->data[address];
            break;

        case FUNCT3_LBU:
            // LBU - 加载无符号字节
            // 示例: lbu x1, 0(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载一个字节，零扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (uint8_t)memory->data[address];
            break;

        case FUNCT3_LHU:
            // LHU - 加载无符号半字
            // 示例: lhu x1, 4(x2)
            // 从 x2 寄存器地址加立即数偏移的内存中加载两个字节，零扩展，存储在 x1 寄存器中
            cpu->registers[rd] = (uint16_t)*(uint16_t *)&memory->data[address];
            break;

        default:
            // 未知指令处理
            printf("Unknown load instruction with funct3: 0x%x\n", funct3);
            break;
    }
}
