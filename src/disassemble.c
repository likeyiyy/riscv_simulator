#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "disassemble.h"
#include "riscv_defs.h"
#include "csr.h"

// 定义寄存器名称
const char* reg_names[] = {
        "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"
};

const char *csr_names[MAX_CSR_INDEX + 1];


void init_csr_names(void) {
    // 初始化所有索引为 "unknown_csr"
    for (int i = 0; i <= MAX_CSR_INDEX; i++) {
        csr_names[i] = "unknown_csr";
    }

    // 使用你的宏名称定义 CSR 名称
    csr_names[CSR_USTATUS] = "ustatus";
    csr_names[CSR_UIE] = "uie";
    csr_names[CSR_UTVEC] = "utvec";
    csr_names[CSR_USCRATCH] = "uscratch";
    csr_names[CSR_UEPC] = "uepc";
    csr_names[CSR_UCAUSE] = "ucause";
    csr_names[CSR_UTVAL] = "utval";
    csr_names[CSR_UIP] = "uip";

    csr_names[CSR_FFLAGS] = "fflags";
    csr_names[CSR_FRM] = "frm";
    csr_names[CSR_FCSR] = "fcsr";

    csr_names[CSR_SSTATUS] = "sstatus";
    csr_names[CSR_SEDELEG] = "sedeleg";
    csr_names[CSR_SIDELEG] = "sideleg";
    csr_names[CSR_SIE] = "sie";
    csr_names[CSR_STVEC] = "stvec";
    csr_names[CSR_SCOUNTEREN] = "scounteren";

    csr_names[CSR_SSCRATCH] = "sscratch";
    csr_names[CSR_SEPC] = "sepc";
    csr_names[CSR_SCAUSE] = "scause";
    csr_names[CSR_STVAL] = "stval";
    csr_names[CSR_SIP] = "sip";
    csr_names[CSR_SATP] = "satp";

    csr_names[CSR_MVENDORID] = "mvendorid";
    csr_names[CSR_MARCHID] = "marchid";
    csr_names[CSR_MIMPID] = "mimpid";
    csr_names[CSR_MHARTID] = "mhartid";

    csr_names[CSR_MSTATUS] = "mstatus";
    csr_names[CSR_MISA] = "misa";
    csr_names[CSR_MEDELEG] = "medeleg";
    csr_names[CSR_MIDELEG] = "mideleg";
    csr_names[CSR_MIE] = "mie";
    csr_names[CSR_MTVEC] = "mtvec";
    csr_names[CSR_MCOUNTEREN] = "mcounteren";

    csr_names[CSR_MSCRATCH] = "mscratch";
    csr_names[CSR_MEPC] = "mepc";
    csr_names[CSR_MCAUSE] = "mcause";
    csr_names[CSR_MTVAL] = "mtval";
    csr_names[CSR_MIP] = "mip";

    csr_names[CSR_MCYCLE] = "mcycle";
    csr_names[CSR_MINSTRET] = "minstret";
    csr_names[CSR_MCYCLEH] = "mcycleh";
    csr_names[CSR_MINSTRETH] = "minstreth";

    csr_names[CSR_VSSTATUS] = "vsstatus";
    csr_names[CSR_VSIE] = "vsie";
    csr_names[CSR_VSTVEC] = "vstvec";
    csr_names[CSR_VSSCRATCH] = "vsscratch";
    csr_names[CSR_VSEPC] = "vsepc";
    csr_names[CSR_VSCAUSE] = "vscause";
    csr_names[CSR_VSTVAL] = "vstval";
    csr_names[CSR_VSIP] = "vsip";

    csr_names[CSR_TSELECT] = "tselect";
    csr_names[CSR_TDATA1] = "tdata1";
    csr_names[CSR_TDATA2] = "tdata2";
    csr_names[CSR_TDATA3] = "tdata3";

    csr_names[CSR_DCSR] = "dcsr";
    csr_names[CSR_DPC] = "dpc";
    csr_names[CSR_DSCRATCH] = "dscratch";
}

const char* get_csr_name(uint32_t addr) {
    if (addr <= MAX_CSR_INDEX) {
        return csr_names[addr];
    }
    return "unknown_csr";
}

//

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

void dis_branch(uint64_t address, uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2,
                int32_t imm);

void dis_jal(uint64_t address, uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, int32_t imm);

void dis_misc_mem(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                  uint32_t rs2, uint32_t funct7);

void dis_system(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                    uint32_t rs2, uint32_t funct7);

void dis_amo(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              uint32_t rs2, uint32_t funct7);

void dis_op_fp(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7);

void disassemble(uint64_t address, uint32_t instruction, char* buffer, size_t buffer_size) {
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
        snprintf(buffer, buffer_size, "lui %s, 0x%x", reg_names[rd], imm);
    } else if (opcode == OPCODE_AUIPC) {  // AUIPC指令
        int32_t imm = instruction & 0xFFFFF000;
        snprintf(buffer, buffer_size, "auipc %s, 0x%x", reg_names[rd], imm);
    } else if (opcode == OPCODE_OP_IMM_32) {
        dis_op_imm_32(instruction, buffer, buffer_size, rd, funct3, rs1, funct7, imm, shamt);
    } else if (opcode == OPCODE_OP_32) {
        dis_op_32(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_BRANCH) {
        dis_branch(address, instruction, buffer, buffer_size, funct3, rs1, rs2, imm);
    } else if (opcode == OPCODE_JAL) {
        dis_jal(address, instruction, buffer, buffer_size, rd, imm);
    } else if (opcode == OPCODE_JALR) {
        imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
        if (imm == 0 && rs1 == 1 && rd == 0) {
            snprintf(buffer, buffer_size, "ret");
        } else {
            snprintf(buffer, buffer_size, "jalr %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
        }
    } else if (opcode == OPCODE_MISC_MEM) {
        dis_misc_mem(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_SYSTEM) {
        dis_system(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_AMO) {
        dis_amo(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else if (opcode == OPCODE_OP_FP) {
        dis_op_fp(instruction, buffer, buffer_size, rd, funct3, rs1, rs2, funct7);
    } else {
        snprintf(buffer, buffer_size, "unknown instruction: 0x%08x", instruction);
    }
}

void dis_misc_mem(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                  uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0: // FENCE
            snprintf(buffer, buffer_size, "fence");
            break;
        case 0x1: // FENCE.I
            snprintf(buffer, buffer_size, "fence.I");
            break;
        default:
            snprintf(buffer, buffer_size, "unknown MISC-MEM instruction: 0x%08x", instruction);
    }
}

void dis_system(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                    uint32_t rs2, uint32_t funct7) {
    uint32_t csr = instruction >> 20;
    uint32_t imm = instruction >> 20;
    switch (funct3) {
        case 0x0: // PRIV
            switch (imm) {
                case OPCODE_ECALL: // ECALL
                    snprintf(buffer, buffer_size, "ecall");
                    break;
                case OPCODE_EBREAK: // EBREAK
                    snprintf(buffer, buffer_size, "ebreak");
                    break;
                case OPCODE_URET: // URET
                    snprintf(buffer, buffer_size, "uret");
                    break;
                case OPCODE_SRET: // SRET
                    snprintf(buffer, buffer_size, "sret");
                    break;
                case OPCODE_MRET: // MRET
                    snprintf(buffer, buffer_size, "mret");
                    break;
                case OPCODE_WFI:
                    snprintf(buffer, buffer_size, "wfi");
                    break;
                default:
                    snprintf(buffer, buffer_size, "unknown PRIV instruction: 0x%08x", instruction);
            }
            break;
        case 0x1: // CSRRW
            snprintf(buffer, buffer_size, "csrrw %s, %s, %s", reg_names[rd], reg_names[rs1], get_csr_name(csr));
            break;
        case 0x2: // CSRRS
            if (rs1 == 0) {
                snprintf(buffer, buffer_size, "csrr %s, %s", reg_names[rd], get_csr_name(csr));
            } else {
                snprintf(buffer, buffer_size, "csrrs %s, %s, %s", reg_names[rd], reg_names[rs1], get_csr_name(csr));
            }
            break;
        case 0x3: // CSRRC
            snprintf(buffer, buffer_size, "csrrc %s, %s, %s", reg_names[rd], reg_names[rs1], get_csr_name(csr));
            break;
        case 0x5: // CSRRWI
            snprintf(buffer, buffer_size, "csrrwi %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        case 0x6: // CSRRSI
            snprintf(buffer, buffer_size, "csrrsi %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        case 0x7: // CSRRCI
            snprintf(buffer, buffer_size, "csrrci %s, %s, %d", reg_names[rd], reg_names[rs1], rs2);
            break;
        default:
            snprintf(buffer, buffer_size, "unknown SYSTEM instruction: 0x%08x", instruction);
    }
}

void dis_amo(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x2: // LR.W
            snprintf(buffer, buffer_size, "lr.W %s, %d(%s)", reg_names[rd], rs2, reg_names[rs1]);
            break;
        case 0x3: // SC.W
            snprintf(buffer, buffer_size, "sc.W %s, %d(%s)", reg_names[rd], rs2, reg_names[rs1]);
            break;
        case 0x1: // AMOSWAP.W
            snprintf(buffer, buffer_size, "amoswap.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x0: // AMOADD.W
            snprintf(buffer, buffer_size, "amoadd.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x4: // AMOXOR.W
            snprintf(buffer, buffer_size, "amoxor.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x8: // AMOAND.W
            snprintf(buffer, buffer_size, "amoand.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0xC: // AMOOR.W
            snprintf(buffer, buffer_size, "amoor.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x10: // AMOMIN.W
            snprintf(buffer, buffer_size, "amomin.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x14: // AMOMAX.W
            snprintf(buffer, buffer_size, "amomax.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x18: // AMOMINU.W
            snprintf(buffer, buffer_size, "amominu.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        case 0x1C: // AMOMAXU.W
            snprintf(buffer, buffer_size, "amomaxu.W %s, %s, %d(%s)", reg_names[rd], reg_names[rs2], rs2, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "unknown AMO instruction: 0x%08x", instruction);
    }
}

void dis_op_fp(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0: // FADD.S
            snprintf(buffer, buffer_size, "fadd.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x1: // FSUB.S
            snprintf(buffer, buffer_size, "fsub.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x2: // FMUL.S
            snprintf(buffer, buffer_size, "fmul.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x3: // FDIV.S
            snprintf(buffer, buffer_size, "fdiv.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x4: // FSGNJ.S
            snprintf(buffer, buffer_size, "fsgnj.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5: // FSGNJN.S
            snprintf(buffer, buffer_size, "fsgnjn.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x6: // FSGNJX.S
            snprintf(buffer, buffer_size, "fsgnjx.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x7: // FMIN.S
            snprintf(buffer, buffer_size, "fmin.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x8: // FMAX.S
            snprintf(buffer, buffer_size, "fmax.S %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0xA: // FSQRT.S
            snprintf(buffer, buffer_size, "fsqrt.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x20: // FCVT.W.S
            snprintf(buffer, buffer_size, "fcvt.W.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x21: // FCVT.WU.S
            snprintf(buffer, buffer_size, "fcvt.WU.S %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x25: // FMV.X.W
            snprintf(buffer, buffer_size, "fmv.X.W %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x61: // FCVT.S.W
            snprintf(buffer, buffer_size, "fcvt.S.W %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        case 0x65: // FMV.W.X
            snprintf(buffer, buffer_size, "fmv.W.X %s, %s", reg_names[rd], reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "unknown FP instruction: 0x%08x", instruction);
    }
}

void dis_jal(uint64_t address, uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, int32_t imm) {
    imm = ((instruction >> 31) << 20) |
      (((instruction >> 21) & 0x3FF) << 1) |
      (((instruction >> 20) & 0x1) << 11) |
      (((instruction >> 12) & 0xFF) << 12);
    imm = (imm << 11) >> 11;  // 符号扩展立即数
    if (rd == 0) {
        snprintf(buffer, buffer_size, "j 0x%x", imm + address);
    } else {
        snprintf(buffer, buffer_size, "jal %s, %d", reg_names[rd], imm);
    }
}

void dis_branch(uint64_t address, uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2,
                int32_t imm) {// B型指令
    imm = (int32_t)(((instruction >> 31) << 12) | (((instruction >> 7) & 0x1) << 11) |
                        (((instruction >> 25) & 0x3F) << 5) | (((instruction >> 8) & 0xF) << 1)) << 19 >> 19; // 符号扩展立即数
    switch (funct3) {
        case FUNCT3_BEQ:
            snprintf(buffer, buffer_size, "beq %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BNE:
            if (rs2 == 0) {
                snprintf(buffer, buffer_size, "bnez %s, 0x%x", reg_names[rs1], address + imm);
            } else {
                snprintf(buffer, buffer_size, "bne %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            }

            break;
        case FUNCT3_BLT:
            snprintf(buffer, buffer_size, "blt %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BGE:
            snprintf(buffer, buffer_size, "bge %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BLTU:
            snprintf(buffer, buffer_size, "bltu %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        case FUNCT3_BGEU:
            snprintf(buffer, buffer_size, "bgeu %s, %s, %d", reg_names[rs1], reg_names[rs2], imm);
            break;
        default:
            snprintf(buffer, buffer_size, "unknown B-type instruction: 0x%08x", instruction);
    }
}

void dis_op_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
               uint32_t rs2, uint32_t funct7) {
    switch (funct3) {
        case 0x0:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "addw %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "subw %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        case 0x1: // SLLW
            snprintf(buffer, buffer_size, "sllw %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "srlw %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "sraw %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        default:
            snprintf(buffer, buffer_size, "unknown R-type extension instruction: 0x%08x", instruction);
    }
}

void dis_op_imm_32(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                   uint32_t funct7, int32_t imm, int32_t shamt) {
    switch (funct3) {
        case 0x0: // ADDIW
            snprintf(buffer, buffer_size, "addiw %s, %s, 0x%x", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x1: // SLLIW
            snprintf(buffer, buffer_size, "slliw %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "srliw %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "sraiw %s, %s, 0x%x", reg_names[rd], reg_names[rs1], shamt);
            }
            break;
        default:
            snprintf(buffer, buffer_size, "unknown I-type extension instruction: 0x%08x", instruction);
    }
}

void dis_store(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t funct3, uint32_t rs1, uint32_t rs2) {
    int32_t imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F); // 提取存储指令的立即数
    imm = (int32_t)(imm << 20) >> 20;  // 符号扩展立即数
    switch (funct3) {
        case 0x0: // SB
            snprintf(buffer, buffer_size, "sb %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x1: // SH
            snprintf(buffer, buffer_size, "sh %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x2: // SW
            snprintf(buffer, buffer_size, "sw %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        case 0x3: // SD
            snprintf(buffer, buffer_size, "sd %s, %d(%s)", reg_names[rs2], imm, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "store: 0x%08x", instruction);
    }
}

void dis_load(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
              int32_t imm) {
    imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
    switch (funct3) {
        case 0x0: // LB
            snprintf(buffer, buffer_size, "lb %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x1: // LH
            snprintf(buffer, buffer_size, "lh %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x2: // LW
            snprintf(buffer, buffer_size, "lw %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x3: // LD
            snprintf(buffer, buffer_size, "ld %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x4: // LBU
            snprintf(buffer, buffer_size, "lbu %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x5: // LHU
            snprintf(buffer, buffer_size, "lhu %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        case 0x6: // LWU
            snprintf(buffer, buffer_size, "lwu %s, %d(%s)", reg_names[rd], imm, reg_names[rs1]);
            break;
        default:
            snprintf(buffer, buffer_size, "load: 0x%08x", instruction);
    }
}

void
dis_op(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1, uint32_t rs2,
       uint32_t funct7) {
    switch (funct3) {
        case 0x0:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "add %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "sub %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        case 0x1: // SLL
            snprintf(buffer, buffer_size, "sll %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x2: // SLT
            snprintf(buffer, buffer_size, "slt %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x3: // SLTU
            snprintf(buffer, buffer_size, "sltu %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x4: // XOR
            snprintf(buffer, buffer_size, "xor %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, buffer_size, "srl %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            } else if (funct7 == 0x20) {
                snprintf(buffer, buffer_size, "sra %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            }
            break;
        case 0x6: // OR
            snprintf(buffer, buffer_size, "or %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        case 0x7: // AND
            snprintf(buffer, buffer_size, "and %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
            break;
        default:
            if (funct7 == FUNCT7_M) {
                switch (funct3) {
                    case FUNCT3_MUL:
                        snprintf(buffer, buffer_size, "mul %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULH:
                        snprintf(buffer, buffer_size, "mulh %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULHSU:
                        snprintf(buffer, buffer_size, "mulhsu %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_MULHU:
                        snprintf(buffer, buffer_size, "mulhu %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_DIV:
                        snprintf(buffer, buffer_size, "div %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_DIVU:
                        snprintf(buffer, buffer_size, "divu %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_REM:
                        snprintf(buffer, buffer_size, "rem %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    case FUNCT3_REMU:
                        snprintf(buffer, buffer_size, "remu %s, %s, %s", reg_names[rd], reg_names[rs1], reg_names[rs2]);
                        break;
                    default:
                        snprintf(buffer, buffer_size, "unknown M-extension instruction: 0x%08x", instruction);
                }
            } else {
                snprintf(buffer, buffer_size, "r: 0x%08x", instruction);
            }

    }
}

void dis_op_imm(uint32_t instruction, char *buffer, size_t buffer_size, uint32_t rd, uint32_t funct3, uint32_t rs1,
                uint32_t funct7, int32_t imm, int32_t shamt) {
    imm = (int32_t)((instruction >> 20) << 20) >> 20;  // 符号扩展立即数
    switch (funct3) {
        case 0x0: // ADDI
            if (rs1 == 0) {
                if (rd == 0) {
                    snprintf(buffer, buffer_size, "nop");
                } else {
                    snprintf(buffer, buffer_size, "li %s, %d", reg_names[rd], imm);
                }
            } else {
                if (imm == 0) {
                    snprintf(buffer, buffer_size, "mv %s, %s", reg_names[rd], reg_names[rs1]);
                } else {
                    snprintf(buffer, buffer_size, "addi %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
                }

            }
            break;
        case 0x1: // SLLI
            snprintf(buffer, buffer_size, "slli %s, %s, 0x%x", reg_names[rd], reg_names[rs1], imm & 0x1F); // 立即数只有低5位有意义
            break;
        case 0x2: // SLTI
            snprintf(buffer, buffer_size, "slti %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x3: // SLTIU
            snprintf(buffer, buffer_size, "sltiu %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x4: // XORI
            snprintf(buffer, buffer_size, "xori %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x5:
            if (funct7 == 0x00) {
                snprintf(buffer, 100, "srli %s, %s, %d", reg_names[rd], reg_names[rs1], shamt);
            } else if (funct7 == 0x20) {
                snprintf(buffer, 100, "srai %s, %s, %d", reg_names[rd], reg_names[rs1], shamt);
            }
            break;
        case 0x6: // ORI
            snprintf(buffer, buffer_size, "ori %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            break;
        case 0x7: // ANDI
            snprintf(buffer, buffer_size, "andi %s, %s, %d", reg_names[rd], reg_names[rs1], imm);
            break;
        default:
            snprintf(buffer, buffer_size, "i: 0x%08x", instruction);
    }
}
