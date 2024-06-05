#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"
#include "memory.h"
void execute_r_type_instruction(CPU *cpu, uint32_t instruction);
void execute_i_type_instruction(CPU *cpu, uint32_t instruction);
void execute_s_type_instruction(CPU *cpu, Memory *memory, uint32_t instruction);
void execute_load_instruction(CPU *cpu, Memory *memory, uint32_t instruction);
// 内联函数声明
static inline void execute_addw(CPU *cpu, uint32_t instruction);
static inline void execute_subw(CPU *cpu, uint32_t instruction);
static inline void execute_sllw(CPU *cpu, uint32_t instruction);
static inline void execute_srlw(CPU *cpu, uint32_t instruction);
static inline void execute_sraw(CPU *cpu, uint32_t instruction);

// 其他内联函数声明
static inline void execute_addiw(CPU *cpu, uint32_t instruction);
static inline void execute_slliw(CPU *cpu, uint32_t instruction);
static inline void execute_srliw(CPU *cpu, uint32_t instruction);
static inline void execute_sraiw(CPU *cpu, uint32_t instruction);
void cpu_init(CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
}

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
            printf("Unknown B-type instruction with funct3: 0x%x\n", funct3);
    }
    cpu->pc += 4;  // 如果没有跳转，则PC增加4
}

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

        cpu->registers[rd] = cpu->pc + 4;
        cpu->pc += imm;
    } else if ((instruction & 0x7F) == OPCODE_JALR) {
        // 处理JALR指令
        imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
        uint32_t rs1 = (instruction >> 15) & 0x1F;

        cpu->registers[rd] = cpu->pc + 4;
        cpu->pc = (cpu->registers[rs1] + imm) & ~1;  // 确保跳转地址是对齐的
    } else {
        printf("Unknown J-type instruction with opcode: 0x%x\n", instruction & 0x7F);
    }
}






void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    bool pc_updated = false;
    cpu->registers[0] = 0;  // 确保x0始终为0

    switch (opcode) {
        case OPCODE_OP: // 处理R型指令
            execute_r_type_instruction(cpu, instruction);
            break;
        case OPCODE_OP_IMM:
            execute_i_type_instruction(cpu, instruction);
            break;
        case OPCODE_STORE:
            execute_s_type_instruction(cpu, memory, instruction);
            break;
        case OPCODE_LOAD:
            // 处理Load指令
            execute_load_instruction(cpu, memory, instruction);
            break;
        case OPCODE_AUIPC:
            // AUIPC指令：rd = pc + imm
            cpu->registers[RD(instruction)] = cpu->pc + (instruction & 0xFFFFF000);
            break;
        case OPCODE_LUI:
            // LUI指令：rd = imm
            cpu->registers[RD(instruction)] = (instruction & 0xFFFFF000);
            break;
        case OPCODE_BRANCH:
            execute_b_type_instruction(cpu, instruction);
            pc_updated = true;  // B型指令已经更新PC
            break;
        case OPCODE_JAL: // 处理JAL指令
        case OPCODE_JALR: // 处理JALR指令
            execute_j_type_instruction(cpu, instruction);
            pc_updated = true;  // J型指令已经更新PC
            break;
        case OPCODE_OP_IMM_32:
            switch ((instruction >> 12) & 0x7) {
                case 0x0: // ADDIW
                    execute_addiw(cpu, instruction);
                    break;
                case 0x1: // SLLIW
                    execute_slliw(cpu, instruction);
                    break;
                case 0x5: // SRLIW 和 SRAIW
                    if ((instruction >> 25) == 0x00) {
                        execute_srliw(cpu, instruction);
                    } else if ((instruction >> 25) == 0x20) {
                        execute_sraiw(cpu, instruction);
                    }
                    break;
                default:
                    printf("Unknown IW-type instruction: 0x%08x\n", instruction);
            }
            break;
        case OPCODE_OP_32:
            switch ((instruction >> 12) & 0x7) {
                case 0x0: // ADDW 和 SUBW
                    if ((instruction >> 25) == 0x00) {
                        execute_addw(cpu, instruction);
                    } else if ((instruction >> 25) == 0x20) {
                        execute_subw(cpu, instruction);
                    }
                    break;
                case 0x1: // SLLW
                    execute_sllw(cpu, instruction);
                    break;
                case 0x5: // SRLW 和 SRAW
                    if ((instruction >> 25) == 0x00) {
                        execute_srlw(cpu, instruction);
                    } else if ((instruction >> 25) == 0x20) {
                        execute_sraw(cpu, instruction);
                    }
                    break;
                default:
                    printf("Unknown RW-type instruction: 0x%08x\n", instruction);
            }
            break;
        default:
            printf("Unknown instruction with opcode: 0x%x\n", opcode);
    }
    if(!pc_updated) {
        cpu->pc += 4;
    }
    cpu->registers[0] = 0;  // 确保x0始终为0
}

// R-type指令处理函数
void execute_r_type_instruction(CPU *cpu, uint32_t instruction) {
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
        default:
            printf("Unknown R-type instruction with funct3: 0x%x\n", funct3);
            break;
    }
}

// I-type指令处理函数
void execute_i_type_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);         // 提取目的寄存器
    uint32_t funct3 = FUNCT3(instruction); // 提取funct3字段
    uint32_t rs1 = RS1(instruction);       // 提取源寄存器1
    int32_t imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
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
    int32_t imm = (int32_t)(((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F)) << 20 >> 20; // 符号扩展立即数


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



static inline void execute_addiw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (instruction >> 20);
    imm = (imm << 20) >> 20;  // 符号扩展立即数
    int32_t result = (int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) + imm;
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_slliw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) << shamt;
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_srliw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t)((uint32_t)(cpu->registers[rs1] & 0xFFFFFFFF) >> shamt);
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_sraiw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t)((int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) >> shamt);
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_addw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) + (int32_t)(cpu->registers[rs2] & 0xFFFFFFFF);
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_subw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) - (int32_t)(cpu->registers[rs2] & 0xFFFFFFFF);
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_sllw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) << (cpu->registers[rs2] & 0x1F);
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_srlw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t)((uint32_t)(cpu->registers[rs1] & 0xFFFFFFFF) >> (cpu->registers[rs2] & 0x1F));
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}

static inline void execute_sraw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t)((int32_t)(cpu->registers[rs1] & 0xFFFFFFFF) >> (cpu->registers[rs2] & 0x1F));
    cpu->registers[rd] = (int64_t)result; // 符号扩展到64位
}
