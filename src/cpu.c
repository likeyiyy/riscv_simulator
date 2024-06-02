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
            // 其他指令类型
            // ...
    }
}
