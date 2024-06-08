#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "disassemble.h"
#include "riscv_defs.h"

// 定义寄存器名称
const char* reg_names[] = {
        "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

void dis_op_imm(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                uint32_t funct7, int32_t imm, int32_t shamt);

void
dis_op(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1, uint32_t rs2,
       uint32_t funct7);

void dis_load(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              int32_t imm);

void dis_store(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2);

void dis_op_imm_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                   uint32_t funct7, int32_t imm, int32_t shamt);

void dis_op_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7);

void dis_branch(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2,
                int32_t imm);

void dis_jal(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, int32_t imm);

void dis_misc_mem(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                  uint32_t rs2, uint32_t funct7);

void dis_system(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                    uint32_t rs2, uint32_t funct7);

void dis_amo(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              uint32_t rs2, uint32_t funct7);

void dis_op_fp(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7);

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
    if (opcode == OPCODE_OP_IMM) {  // I型指令
        dis_op_imm(instruction, buffer, buffer_size, rd, funct3, rs1, funct7, imm, shamt);
    } else if (opcode == OPCODE_OP) {  // R型指令
        dis_op(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_LOAD) {  // LOAD指令
        dis_load(instruction, buffer, buffer_size, rd, funct3, rs1, imm);
    } else if (opcode == OPCODE_STORE) {  // STORE指令
        dis_store(instruction, buffer, buffer_size, funct3, rs1, rs2);
    } else if (opcode == OPCODE_LUI) {  // LUI指令
        int32_t imm = instruction & 0xFFFFF000;
        snprintf(buffer, buffer_size, "LUI %s, 0x%x", reg_names[rd], imm);
    } else if (opcode == OPCODE_AUIPC) {  // AUIPC指令
        int32_t imm = instruction & 0xFFFFF000;
        snprintf(buffer, buffer_size, "AUIPC %s, 0x%x", reg_names[rd], imm);
    } else if (opcode == OPCODE_OP_IMM_32) {
        dis_op_imm_32(instruction, buffer, buffer_size, rd, funct3, rs1, funct7, imm, shamt);
    } else if (opcode == OPCODE_OP_32) {
        dis_op_32(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_BRANCH) {
        dis_branch(instruction, buffer, buffer_size, funct3, rs1, rs2, imm);
    } else if (opcode == OPCODE_JAL) {
        dis_jal(instruction, buffer, buffer_size, rd, imm);
    } else if (opcode == OPCODE_JALR) {
        imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
        snprintf(buffer, buffer_size, "JALR %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
    } else if (opcode == OPCODE_MISC_MEM) {
        dis_misc_mem(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_SYSTEM) {
        dis_system(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_AMO) {
        dis_amo(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_OP_FP) {
        dis_op_fp(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else {
        snprintf(buffer, buffer_size, "Unknown instruction: 0x%08x", instruction);
    }
}

void dis_misc_mem(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                  uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0: // FENCE
            snprintf(buffer, buffer_size, "FENCE");
            break;
        case 0x1: // FENCE.I
            snprintf(buffer, buffer_size, "FENCE.I");
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown MISC-MEM instruction: 0x%08x", instruction);
    }
}

void dis_system(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                    uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0: // PRIV
            snprintf(buffer, buffer_size, "ECALL");
            break;
        case 0x1: // CSRRW
            snprintf(buffer, buffer_size, "CSRRW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x2: // CSRRS
            snprintf(buffer, buffer_size, "CSRRS %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x3: // CSRRC
            snprintf(buffer, buffer_size, "CSRRC %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5: // CSRRWI
            snprintf(buffer, buffer_size, "CSRRWI %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        case 0x6: // CSRRSI
            snprintf(buffer, buffer_size, "CSRRSI %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        case 0x7: // CSRRCI
            snprintf(buffer, buffer_size, "CSRRCI %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown SYSTEM instruction: 0x%08x", instruction);
    }
}

void dis_amo(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x2: // LR.W
            snprintf(buffer, buffer_size, "LR.W %s, %d(%s)", reg_names[rd], rs2, reg_names[rs1]);
            break;
        case 0x3: // SC.W
            snprintf(buffer, buffer_size, "SC.W %s, %d(%s)", reg_names[rd], rs2, reg_names[rs1]);
            break;
        case 0x1: // AMOSWAP.W
            snprintf(buffer, buffer_size, "AMOSWAP.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x0: // AMOADD.W
            snprintf(buffer, buffer_size, "AMOADD.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x4: // AMOXOR.W
            snprintf(buffer, buffer_size, "AMOXOR.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x8: // AMOAND.W
            snprintf(buffer, buffer_size, "AMOAND.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0xC: // AMOOR.W
            snprintf(buffer, buffer_size, "AMOOR.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x10: // AMOMIN.W
            snprintf(buffer, buffer_size, "AMOMIN.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x14: // AMOMAX.W
            snprintf(buffer, buffer_size, "AMOMAX.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x18: // AMOMINU.W
            snprintf(buffer, buffer_size, "AMOMINU.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x1C: // AMOMAXU.W
            snprintf(buffer, buffer_size, "AMOMAXU.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown AMO instruction: 0x%08x", instruction);
    }
}

void dis_op_fp(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0: // FADD.S
            snprintf(buffer, buffer_size, "FADD.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x1: // FSUB.S
            snprintf(buffer, buffer_size, "FSUB.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x2: // FMUL.S
            snprintf(buffer, buffer_size, "FMUL.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x3: // FDIV.S
            snprintf(buffer, buffer_size, "FDIV.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x4: // FSGNJ.S
            snprintf(buffer, buffer_size, "FSGNJ.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5: // FSGNJN.S
            snprintf(buffer, buffer_size, "FSGNJN.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x6: // FSGNJX.S
            snprintf(buffer, buffer_size, "FSGNJX.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x7: // FMIN.S
            snprintf(buffer, buffer_size, "FMIN.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x8: // FMAX.S
            snprintf(buffer, buffer_size, "FMAX.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0xA: // FSQRT.S
            snprintf(buffer, buffer_size, "FSQRT.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x20: // FCVT.W.S
            snprintf(buffer, buffer_size, "FCVT.W.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x21: // FCVT.WU.S
            snprintf(buffer, buffer_size, "FCVT.WU.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x25: // FMV.X.W
            snprintf(buffer, buffer_size, "FMV.X.W %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x61: // FCVT.S.W
            snprintf(buffer, buffer_size, "FCVT.S.W %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x65: // FMV.W.X
            snprintf(buffer, buffer_size, "FMV.W.X %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown FP instruction: 0x%08x", instruction);
    }
}

void dis_jal(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, int32_t imm) {
    imm = ((instruction >> 31) << 20) |
      (((instruction >> 21) & 0x3FF) << 1) |
      (((instruction >> 20) & 0x1) << 11) |
      (((instruction >> 12) & 0xFF) << 12);
    imm = (imm << 11) >> 11;  // 符号扩展立即数
    if (rd == 0) {
        snprintf(buffer, buffer_size, "J %d", imm);
    } else {
        snprintf(buffer, buffer_size, "JAL %s, %d", reg_names[rd], imm);
    }
}

void dis_branch(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2,
                int32_t imm) {// B型指令
    imm = (int32_t)(((instruction >> 31) << 12) | (((instruction >> 7) & 0x1) << 11) |
                        (((instruction >> 25) & 0x3F) << 5) | (((instruction >> 8) & 0xF) << 1)) << 19 >> 19; // 符号扩展立即数
    switch (funct3) {
        case FUNCT3_BEQ:
            snprintf(buffer, buffer_size, "BEQ %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BNE:
            snprintf(buffer, buffer_size, "BNE %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BLT:
            snprintf(buffer, buffer_size, "BLT %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BGE:
            snprintf(buffer, buffer_size, "BGE %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BLTU:
            snprintf(buffer, buffer_size, "BLTU %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BGEU:
            snprintf(buffer, buffer_size, "BGEU %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown B-type instruction: 0x%08x", instruction);
    }
}

void dis_op_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "ADDW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "SUBW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        case 0x1: // SLLW
            snprintf(buffer, buffer_size, "SLLW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "SRLW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "SRAW %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown R-type extension instruction: 0x%08x", instruction);
    }
}

void dis_op_imm_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                   uint32_t funct7, int32_t imm, int32_t shamt) {
    switch (funct3) {
        case 0x0: // ADDIW
            snprintf(buffer, buffer_size, "ADDIW %s, %s, 0x%x", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x1: // SLLIW
            snprintf(buffer, buffer_size, "SLLIW %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "SRLIW %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "SRAIW %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            }
            break;
        default:
            snprintf(buffer, buffer_size, "Unknown I-type extension instruction: 0x%08x", instruction);
    }
}

void dis_store(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2) {
    int32_t imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F); // 提取存储指令的立即数
    switch (funct3) {
        case 0x0: // SB
            snprintf(buffer, buffer_size, "SB %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x1: // SH
            snprintf(buffer, buffer_size, "SH %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x2: // SW
            snprintf(buffer, buffer_size, "SW %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x3: // SD
            snprintf(buffer, buffer_size, "SD %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "STORE: 0x%08x", instruction);
    }
}

void dis_load(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              int32_t imm) {
    switch (funct3) {
        case 0x0: // LB
            snprintf(buffer, buffer_size, "LB %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x1: // LH
            snprintf(buffer, buffer_size, "LH %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x2: // LW
            snprintf(buffer, buffer_size, "LW %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x3: // LD
            snprintf(buffer, buffer_size, "LD %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x4: // LBU
            snprintf(buffer, buffer_size, "LBU %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x5: // LHU
            snprintf(buffer, buffer_size, "LHU %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x6: // LWU
            snprintf(buffer, buffer_size, "LWU %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "LOAD: 0x%08x", instruction);
    }
}

void
dis_op(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1, uint32_t rs2,
       uint32_t funct7) {
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
            if (funct7 == FUNCT7_M) {
                switch (funct3) {
                    case FUNCT3_MUL:
                        snprintf(buffer, buffer_size, "MUL %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULH:
                        snprintf(buffer, buffer_size, "MULH %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULHSU:
                        snprintf(buffer, buffer_size, "MULHSU %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULHU:
                        snprintf(buffer, buffer_size, "MULHU %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_DIV:
                        snprintf(buffer, buffer_size, "DIV %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_DIVU:
                        snprintf(buffer, buffer_size, "DIVU %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_REM:
                        snprintf(buffer, buffer_size, "REM %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_REMU:
                        snprintf(buffer, buffer_size, "REMU %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    default:
                        snprintf(buffer, buffer_size, "Unknown M-extension instruction: 0x%08x", instruction);
                }
            } else {
                snprintf(buffer, buffer_size, "R: 0x%08x", instruction);
            }

    }
}

void dis_op_imm(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                uint32_t funct7, int32_t imm, int32_t shamt) {
    imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
    switch (funct3) {
        case 0x0: // ADDI
            if (rs1 == 0) {
                snprintf(buffer, buffer_size, "LI %s, %d", reg_names[rd], imm);
            } else {
                snprintf(buffer, buffer_size, "ADDI %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            }
            break;
        case 0x1: // SLLI
            snprintf(buffer, buffer_size, "SLLI %s, %s, 0x%x", reg_names[rd], reg_names[rs1], imm & 0x1F); // 立即数只有低5位有意义
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
}
