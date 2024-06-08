#include "f_extension.h"
#include "csr.h"
#include <math.h>

// F 扩展指令实现
inline void execute_fadd_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = src1 + src2;
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fsub_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = src1 - src2;
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fmul_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = src1 * src2;
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fdiv_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = src1 / src2;
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fsqrt_s(CPU *cpu, uint32_t rd, uint32_t rs1) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float result = sqrtf(src1);
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fsgnj_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t src1 = (uint32_t)cpu->fregisters[rs1];
    uint32_t src2 = (uint32_t)cpu->fregisters[rs2];
    uint32_t result = (src1 & 0x7FFFFFFF) | (src2 & 0x80000000);
    cpu->fregisters[rd] = result;
}

inline void execute_fsgnjn_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t src1 = (uint32_t)cpu->fregisters[rs1];
    uint32_t src2 = (uint32_t)cpu->fregisters[rs2];
    uint32_t result = (src1 & 0x7FFFFFFF) | (~src2 & 0x80000000);
    cpu->fregisters[rd] = result;
}

inline void execute_fsgnjx_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t src1 = (uint32_t)cpu->fregisters[rs1];
    uint32_t src2 = (uint32_t)cpu->fregisters[rs2];
    uint32_t result = src1 ^ (src2 & 0x80000000);
    cpu->fregisters[rd] = result;
}

inline void execute_fmin_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = fminf(src1, src2);
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

inline void execute_fmax_s(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    float src1 = *((float*)&cpu->fregisters[rs1]);
    float src2 = *((float*)&cpu->fregisters[rs2]);
    float result = fmaxf(src1, src2);
    cpu->fregisters[rd] = *((uint64_t*)&result);
}

void execute_f_extension_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;

    if (funct7 == FUNCT7_F) {
        switch (funct3) {
            case FUNCT3_FADD_S:
                execute_fadd_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FSUB_S:
                execute_fsub_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FMUL_S:
                execute_fmul_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FDIV_S:
                execute_fdiv_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FSQRT_S:
                execute_fsqrt_s(cpu, rd, rs1);
                break;
            case FUNCT3_FSGNJ_S:
                execute_fsgnj_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FSGNJN_S:
                execute_fsgnjn_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FSGNJX_S:
                execute_fsgnjx_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FMIN_S:
                execute_fmin_s(cpu, rd, rs1, rs2);
                break;
            case FUNCT3_FMAX_S:
                execute_fmax_s(cpu, rd, rs1, rs2);
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    } else {
        raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
    }
}
