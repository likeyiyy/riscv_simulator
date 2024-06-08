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


void cpu_init(CPU *cpu, Memory *memory) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
    cpu->priv = PRV_M;
    cpu->memory = memory;
}

void handle_interrupt(CPU *cpu) {
    // 检查软件中断
    if (cpu->clint.msip & 1) {
        // 清除软件中断
        cpu->clint.msip &= ~1;
        // 处理软件中断
        raise_exception(cpu, CAUSE_MACHINE_SOFTWARE_INTERRUPT);
    }

    // 检查定时器中断
    if (cpu->clint.mtime >= cpu->clint.mtimecmp) {
        // 清除定时器中断
        clear_timer_interrupt(&cpu->clint);
        // 处理定时器中断
        raise_exception(cpu, CAUSE_MACHINE_TIMER_INTERRUPT);
    }

    // 检查外部中断
    uint32_t interrupt_id = claim_interrupt(&cpu->plic);
    if (interrupt_id != 0) {
        // 处理外部中断
        raise_exception(cpu, CAUSE_MACHINE_EXTERNAL_INTERRUPT);
        // 完成中断处理
        complete_interrupt(&cpu->plic, interrupt_id);
    }
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




void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction) {
    uint32_t opcode = OPCODE(instruction); // 提取操作码
    // 检查并处理中断
    handle_interrupt(cpu);

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
            printf("Unknown instruction with opcode: 0x%x\n", opcode);
    }
    if(!pc_updated) {
        cpu->pc += 4;
    }
    cpu->registers[0] = 0;  // 确保x0始终为0
}

