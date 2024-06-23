#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"
#include "memory.h"
#include "fence_inst.h"
#include "a_extension.h"
#include "f_extension.h"
#include "r_inst.h"
#include "i_inst.h"
#include "s_inst.h"
#include "l_inst.h"
#include "b_inst.h"
#include "j_inst.h"
#include "csr.h"
#include "clint.h"
#include "plic.h"
#include "i_64_inst.h"
#include "mfprintf.h"
#include "exception.h"

static CPU global_cpu;

CPU *get_cpu(void) {
    return &global_cpu;
}

void cpu_init(CPU *cpu, Memory *memory, CLINT *clint, PLIC *plic, UART *uart) {
    cpu->pc = 0;
    cpu->priv = PRV_M;
    cpu->memory = memory;
    cpu->interrupt_pending = false;
    cpu->fast_mode = false;
    memset(cpu->registers, 0, sizeof(cpu->registers));
    memset(cpu->fregisters, 0, sizeof(cpu->fregisters));
    memset(cpu->csr, 0, sizeof(cpu->csr));
    init_mmu(&cpu->mmu);
    // 初始化中断优先级
    cpu->current_priority = 0;
    cpu->clint = clint;
    cpu->plic = plic;
    cpu->uart = uart;
}


void trigger_interrupt(CPU *cpu, int interrupt_id) {
    PLIC *plic = get_plic();
    mfprintf("Keyboard Trigger\n");
    plic->pending[interrupt_id >> 5] |= (1 << (interrupt_id & 0x1F));
    cpu->csr[CSR_MIP] |= MIP_MEIP;
    mfprintf("Keyboard Trigger interrupt %d, cpu->csr[CSR_MIP]: 0x%x, plic->pending[0]: 0x%x\n",
             interrupt_id,
             cpu->csr[CSR_MIP],
             plic->pending[interrupt_id >> 5]
    );
}


void cpu_execute(CPU *cpu, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    cpu->registers[0] = 0;  // 确保x0始终为0


    cpu->pc_updated = false;

    int32_t imm;
    int64_t offset;
    uint64_t result;

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
            // 提取高20位的立即数部分
            imm = (int32_t) ((instruction >> 12) << 12) >> 12;  // 符号扩展立即数
            // 左移12位，进行符号扩展
            offset = (int64_t) imm << 12;
            // 计算目标地址
            result = cpu->pc + offset;
            cpu->registers[RD(instruction)] = result;
            break;
        case OPCODE_LUI:
            // LUI指令：rd = imm
            cpu->registers[RD(instruction)] = (instruction & 0xFFFFF000);
            break;
        case OPCODE_BRANCH:
            execute_b_type_instruction(cpu, instruction);
            cpu->pc_updated = true;  // B型指令已经更新PC
            break;
        case OPCODE_JAL: // 处理JAL指令
        case OPCODE_JALR: // 处理JALR指令
            execute_j_type_instruction(cpu, instruction);
            cpu->pc_updated = true;  // J型指令已经更新PC
            break;
        case OPCODE_OP_IMM_32:
            execute_i_32_type_instruction(cpu, instruction);
            break;
        case OPCODE_OP_32:
            execute_r_32_type_instruction(cpu, instruction);
            break;
        case OPCODE_MISC_MEM:
            execute_misc_mem_instructions(cpu, instruction);
            break;
        case OPCODE_SYSTEM:
            execute_system_instruction(cpu, instruction);
            break;
        case OPCODE_AMO:
            execute_atomic_instruction(cpu, instruction);
            break;
        case OPCODE_OP_FP:  // 假设浮点指令的操作码定义为 OPCODE_OP_FP
            execute_f_extension_instruction(cpu, instruction);
            break;
        default:
            mfprintf("Unknown instruction with opcode: 0x%x\n", opcode);
    }
    if (!cpu->pc_updated) {
        cpu->pc += 4;
    }
    cpu->registers[0] = 0;  // 确保x0始终为0
    // 检查并处理中断
    handle_interrupt(cpu);
}

