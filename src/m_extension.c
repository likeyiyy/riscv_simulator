#include "m_extension.h"
#include "csr.h"
#include "exception.h"

// M 扩展指令实现
inline void execute_mul(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int64_t src1 = (int64_t) cpu->registers[rs1];
    int64_t src2 = (int64_t) cpu->registers[rs2];
    cpu->registers[rd] = (uint64_t) (src1 * src2);
}

inline void execute_mulh(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int64_t src1 = (int64_t) cpu->registers[rs1];
    int64_t src2 = (int64_t) cpu->registers[rs2];
    __int128_t result = (__int128_t) src1 * (__int128_t) src2;
    cpu->registers[rd] = (uint64_t) (result >> 64);
}

inline void execute_mulhsu(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int64_t src1 = (int64_t) cpu->registers[rs1];
    uint64_t src2 = cpu->registers[rs2];
    __int128_t result = (__int128_t) src1 * (__int128_t) src2;
    cpu->registers[rd] = (uint64_t) (result >> 64);
}

inline void execute_mulhu(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t src1 = cpu->registers[rs1];
    uint64_t src2 = cpu->registers[rs2];
    __uint128_t result = (__uint128_t) src1 * (__uint128_t) src2;
    cpu->registers[rd] = (uint64_t) (result >> 64);
}

inline void execute_div(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int64_t src1 = (int64_t) cpu->registers[rs1];
    int64_t src2 = (int64_t) cpu->registers[rs2];
    if (src2 == 0) {
        cpu->registers[rd] = -1; // Division by zero
    } else if (src1 == INT64_MIN && src2 == -1) {
        cpu->registers[rd] = src1; // Overflow
    } else {
        cpu->registers[rd] = src1 / src2;
    }
}

inline void execute_divu(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t src1 = cpu->registers[rs1];
    uint64_t src2 = cpu->registers[rs2];
    if (src2 == 0) {
        cpu->registers[rd] = UINT64_MAX; // Division by zero
    } else {
        cpu->registers[rd] = src1 / src2;
    }
}

inline void execute_rem(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int64_t src1 = (int64_t) cpu->registers[rs1];
    int64_t src2 = (int64_t) cpu->registers[rs2];
    if (src2 == 0) {
        cpu->registers[rd] = src1; // Division by zero
    } else if (src1 == INT64_MIN && src2 == -1) {
        cpu->registers[rd] = 0; // Overflow
    } else {
        cpu->registers[rd] = src1 % src2;
    }
}

inline void execute_remu(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t src1 = cpu->registers[rs1];
    uint64_t src2 = cpu->registers[rs2];
    if (src2 == 0) {
        cpu->registers[rd] = src1; // Division by zero
    } else {
        cpu->registers[rd] = src1 % src2;
    }
}

void execute_m_extension_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;

    if (funct7 == FUNCT7_M) {
        switch (funct3) {
            case FUNCT3_MUL:
                execute_mul(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_MULH:
                execute_mulh(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_MULHSU:
                execute_mulhsu(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_MULHU:
                execute_mulhu(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_DIV:
                execute_div(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_DIVU:
                execute_divu(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_REM:
                execute_rem(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_REMU:
                execute_remu(cpu, rd, rs1, rs2);
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    } else {
        raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }
}
