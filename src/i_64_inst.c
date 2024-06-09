#include "i_64_inst.h"
#include "csr.h"

static inline void execute_addiw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (instruction >> 20);
    imm = (imm << 20) >> 20;  // 符号扩展立即数
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) + imm;
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_slliw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) << shamt;
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_srliw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t) ((uint32_t) (cpu->registers[rs1] & 0xFFFFFFFF) >> shamt);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_sraiw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t shamt = (instruction >> 20) & 0x1F;  // 提取移位量
    int32_t result = (int32_t) ((int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) >> shamt);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_addw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) + (int32_t) (cpu->registers[rs2] & 0xFFFFFFFF);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_subw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) - (int32_t) (cpu->registers[rs2] & 0xFFFFFFFF);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_sllw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) << (cpu->registers[rs2] & 0x1F);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_srlw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) ((uint32_t) (cpu->registers[rs1] & 0xFFFFFFFF) >> (cpu->registers[rs2] & 0x1F));
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_sraw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) ((int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) >> (cpu->registers[rs2] & 0x1F));
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_mulw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    int32_t result = (int32_t) (cpu->registers[rs1] & 0xFFFFFFFF) * (int32_t) (cpu->registers[rs2] & 0xFFFFFFFF);
    cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
}

static inline void execute_divw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    int32_t rs1 = (int32_t) (cpu->registers[(instruction >> 15) & 0x1F] & 0xFFFFFFFF);
    int32_t rs2 = (int32_t) (cpu->registers[(instruction >> 20) & 0x1F] & 0xFFFFFFFF);
    if (rs2 == 0) {
        cpu->registers[rd] = -1; // 除数为0，商为-1
    } else if (rs1 == 0x80000000 && rs2 == -1) {
        cpu->registers[rd] = (int64_t) rs1; // 溢出，商为被除数
    } else {
        int32_t result = rs1 / rs2;
        cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
    }
}

static inline void execute_divuw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (uint32_t) (cpu->registers[(instruction >> 15) & 0x1F] & 0xFFFFFFFF);
    uint32_t rs2 = (uint32_t) (cpu->registers[(instruction >> 20) & 0x1F] & 0xFFFFFFFF);
    if (rs2 == 0) {
        cpu->registers[rd] = 0xFFFFFFFF; // 除数为0，商为0xFFFFFFFF
    } else {
        uint32_t result = rs1 / rs2;
        cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
    }
}

static inline void execute_remw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    int32_t rs1 = (int32_t) (cpu->registers[(instruction >> 15) & 0x1F] & 0xFFFFFFFF);
    int32_t rs2 = (int32_t) (cpu->registers[(instruction >> 20) & 0x1F] & 0xFFFFFFFF);
    if (rs2 == 0) {
        cpu->registers[rd] = rs1; // 除数为0，余数为被除数
    } else if (rs1 == 0x80000000 && rs2 == -1) {
        cpu->registers[rd] = 0; // 溢出，余数为0
    } else {
        int32_t result = rs1 % rs2;
        cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
    }
}

static inline void execute_remuw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (uint32_t) (cpu->registers[(instruction >> 15) & 0x1F] & 0xFFFFFFFF);
    uint32_t rs2 = (uint32_t) (cpu->registers[(instruction >> 20) & 0x1F] & 0xFFFFFFFF);
    if (rs2 == 0) {
        cpu->registers[rd] = rs1; // 除数为0，余数为被除数
    } else {
        uint32_t result = rs1 % rs2;
        cpu->registers[rd] = (int64_t) result; // 符号扩展到64位
    }
}

void execute_i_32_type_instruction(CPU *cpu, uint32_t instruction) {

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
            raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }
}

void execute_r_32_type_instruction(CPU *cpu, uint32_t instruction) {
    uint16_t funct7 = (instruction >> 25) & 0x7F;
    uint16_t funct3 = (instruction >> 12) & 0x7;
    if (funct7 == 0x1) {
        switch (funct3) {
            case 0x0: // MULW
                execute_mulw(cpu, instruction);
                break;
            case 0x4: // DIVW
                execute_divw(cpu, instruction);
                break;
            case 0x5: // DIVUW
                execute_divuw(cpu, instruction);
                break;
            case 0x6: // REMW
                execute_remw(cpu, instruction);
                break;
            case 0x7: // REMUW
                execute_remuw(cpu, instruction);
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
        }
    } else {
        switch (funct3) {
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
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
        }
    }

}