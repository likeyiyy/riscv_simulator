#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#include "cpu.h"
#include "memory.h"
#include "a_extension.h"
#include "f_extension.h"
#include "r_inst.h"
#include "i_inst.h"
#include "s_inst.h"
#include "load_inst.h"
#include "b_inst.h"
#include "j_inst.h"


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
void cpu_init(CPU *cpu, Memory *memory) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
    cpu->priv = PRV_M;
    cpu->memory = memory;
}


void execute_fence(CPU *cpu, uint32_t instruction) {
    // FENCE 指令不需要实际的操作，在模拟器中可以作为屏障
    atomic_thread_fence(memory_order_seq_cst);
}

void execute_fence_i(CPU *cpu, uint32_t instruction) {
    // FENCE.I 指令在模拟器中通常不需要实际操作
    // 在实际硬件上，这会刷新指令缓存
}

void execute_fence_tso(CPU *cpu, uint32_t instruction) {
    atomic_thread_fence(memory_order_seq_cst);
}

void raise_exception(CPU *cpu, uint64_t cause) {
    switch (cpu->priv) {
        case PRV_U:
            // 垂直陷入到超级模式
            cpu->csr[CSR_UEPC] = cpu->pc;
            cpu->csr[CSR_UCAUSE] = cause;
            cpu->pc = cpu->csr[CSR_STVEC];
            cpu->priv = PRV_S; // 切换到超级模式
            break;
        case PRV_S:
            // 水平陷入处理
            cpu->csr[CSR_SEPC] = cpu->pc;
            cpu->csr[CSR_SCAUSE] = cause;
            cpu->pc = cpu->csr[CSR_STVEC];
            break;
        case PRV_M:
        default:
            cpu->csr[CSR_MEPC] = cpu->pc;
            cpu->csr[CSR_MCAUSE] = cause;
            cpu->pc = cpu->csr[CSR_MTVEC];
            break;
    }
}


void execute_ecall(CPU *cpu) {
    uint64_t cause;
    switch (cpu->priv) {
        case PRV_U:
            cause = CAUSE_USER_ECALL;
            break;
        case PRV_S:
            cause = CAUSE_SUPERVISOR_ECALL;
            break;
        case PRV_M:
        default:
            // 处理不支持的特权级
            cause = CAUSE_MACHINE_ECALL; // 默认为机器模式
            break;
    }
    raise_exception(cpu, cause);
}

void execute_ebreak(CPU *cpu) {
    raise_exception(cpu, 0x3); // 断点异常
}


void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    uint32_t funct3 = (instruction >> 12) & 0x7;

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
            execute_s_type_instruction(cpu, instruction);
            break;
        case OPCODE_LOAD:
            // 处理Load指令
            execute_load_instruction(cpu, instruction);
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
        case OPCODE_MISC_MEM:
            if ((instruction & 0x07000000) == 0x00000000) {
                execute_fence(cpu, instruction);
            } else if ((instruction & 0x07000000) == 0x01000000) {
                execute_fence_i(cpu, instruction);
            } else if ((instruction & 0xF00FFFFF) == 0x8000000F) {
                execute_fence_tso(cpu, instruction);
            }
            break;
        case OPCODE_SYSTEM:
            if (funct3 == 0) {
                uint32_t funct12 = instruction >> 20;
                if (funct12 == 0) {
                    execute_ecall(cpu);
                } else if (funct12 == 1) {
                    execute_ebreak(cpu);
                }
            }
            break;
        case OPCODE_AMO:
            execute_atomic_instruction(cpu, instruction);
            break;
        case OPCODE_OP_FP:  // 假设浮点指令的操作码定义为 OPCODE_OP_FP
            execute_f_extension_instruction(cpu, instruction);
            break;
        default:
            printf("Unknown instruction with opcode: 0x%x\n", opcode);
    }
    if(!pc_updated) {
        cpu->pc += 4;
    }
    cpu->registers[0] = 0;  // 确保x0始终为0
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
