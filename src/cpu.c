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
#include "load_inst.h"
#include "b_inst.h"
#include "j_inst.h"
#include "csr.h"
#include "clint.h"
#include "plic.h"
#include "i_64_inst.h"
#include "display.h"
#include "mfprintf.h"


void cpu_init(CPU *cpu, Memory *memory, UART *uart) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
    cpu->priv = PRV_M;
    cpu->memory = memory;
    cpu->interrupt_pending = false;
    cpu->fast_mode = false;
    init_mmu(&cpu->mmu);
    // 初始化 CLINT 和 PLIC
    init_clint(&cpu->clint, cpu->cpu_id, memory);
    init_plic(&cpu->plic);
    // 初始化中断优先级
    cpu->current_priority = 0;
    cpu->uart = uart;
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

void handle_interrupt(CPU *cpu) {
    uint64_t status;
    uint64_t ie;  // Interrupt Enable register
    uint64_t ip;  // Interrupt Pending register


    // 根据当前特权级别选择状态寄存器和中断使能寄存器
    switch (cpu->priv) {
        case PRV_M:
            status = cpu->csr[CSR_MSTATUS];
            ie = cpu->csr[CSR_MIE];
            ip = cpu->csr[CSR_MIP];
            break;
        case PRV_S:
            status = cpu->csr[CSR_SSTATUS];
            ie = cpu->csr[CSR_SIE];
            ip = cpu->csr[CSR_SIP];
            break;
        case PRV_U:
            status = cpu->csr[CSR_USTATUS];
            ie = cpu->csr[CSR_UIE];
            ip = cpu->csr[CSR_UIP];
            break;
        default:
            // 未知的特权级别，直接返回
            return;
    }

    if (cpu->priv == PRV_M && (status & MSTATUS_MIE) == 0) {
        // 如果中断未使能，直接返回
        return;
    }

    // 检查并处理高优先级中断
    // 处理器在每个时钟周期都会检查 CSR_MIP 寄存器的状态，以决定是否有中断需要处理。
    // 如果一个挂起位被设置，并且相应的中断使能位也被设置，那么处理器会触发中断处理流程。
    if ((ie & MIE_MEIE) && (ip & MIP_MEIP) && cpu->current_priority < PRIORITY_MACHINE_EXTERNAL_INTERRUPT) {
        cpu->current_priority = PRIORITY_MACHINE_EXTERNAL_INTERRUPT;
        // 处理外部中断
        uint32_t interrupt_id = claim_interrupt(&cpu->plic);
        if (interrupt_id != 0) {
            raise_exception(cpu, CAUSE_MACHINE_EXTERNAL_INTERRUPT);
            complete_interrupt(&cpu->plic, interrupt_id);
        }
    }

    // 检查并处理中优先级中断
    if ((ie & MIE_MTIE) && (*cpu->clint.mtime >= *cpu->clint.mtimecmp) && cpu->current_priority < PRIORITY_MACHINE_TIMER_INTERRUPT) {
        cpu->current_priority = PRIORITY_MACHINE_TIMER_INTERRUPT;
        // 处理定时器中断
        raise_exception(cpu, CAUSE_MACHINE_TIMER_INTERRUPT);
    }

    // 检查并处理低优先级中断
    if ((ie & MIE_MSIE) && (ip & MIP_MSIP) && cpu->current_priority < PRIORITY_MACHINE_SOFTWARE_INTERRUPT) {
        cpu->current_priority = PRIORITY_MACHINE_SOFTWARE_INTERRUPT;
        // 清除软件中断挂起位
        cpu->csr[CSR_MIP] &= ~MIP_MSIP;
        // 处理软件中断
        raise_exception(cpu, CAUSE_MACHINE_SOFTWARE_INTERRUPT);
    }
}


void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    // 检查并处理中断
    // handle_interrupt(cpu);

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
    if(!pc_updated) {
        cpu->pc += 4;
    }
    cpu->registers[0] = 0;  // 确保x0始终为0
}

