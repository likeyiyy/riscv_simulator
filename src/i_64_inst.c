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
            raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }
}