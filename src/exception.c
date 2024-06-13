#include "csr.h"
#include "exception.h"
#include "mfprintf.h"

void raise_exception(CPU *cpu, uint64_t cause) {
    switch (cpu->priv) {
        case PRV_U:
            // 垂直陷入到超级模式
            cpu->csr[CSR_MEPC] = cpu->pc;
            cpu->csr[CSR_MCAUSE] = cause;
            cpu->pc = cpu->csr[CSR_MTVEC];
            // save cpu->priv first
            cpu->csr[CSR_MSTATUS] = (cpu->csr[CSR_MSTATUS] & ~MSTATUS_MPP) | (cpu->priv << 11);
            // save interrupt enable first
            cpu->csr[CSR_MSTATUS] =
                    (cpu->csr[CSR_MSTATUS] & ~MSTATUS_MPIE) | (((cpu->csr[CSR_MSTATUS] >> 3) & 0x1) << 7);
            cpu->priv = PRV_M; // 切换到超级模式
            cpu->pc_updated = true;
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
            // save cpu->priv first
            cpu->csr[CSR_MSTATUS] = (cpu->csr[CSR_MSTATUS] & ~MSTATUS_MPP) | (cpu->priv << 11);
            // save interrupt enable first
            cpu->csr[CSR_MSTATUS] =
                    (cpu->csr[CSR_MSTATUS] & ~MSTATUS_MPIE) | (((cpu->csr[CSR_MSTATUS] >> 3) & 0x1) << 7);
            cpu->pc_updated = true;

            break;
    }
}


inline bool handle_interrupt(CPU *cpu) {
    uint64_t status;
    uint64_t ie;  // Interrupt Enable register
    uint64_t ip;  // Interrupt Pending register

    status = cpu->csr[CSR_MSTATUS];
    ie = cpu->csr[CSR_MIE];
    ip = cpu->csr[CSR_MIP];

    if ((status & MSTATUS_MIE) == 0) {
        // 如果中断未使能，直接返回
        return false;
    }

    // 检查并处理高优先级中断
    // 处理器在每个时钟周期都会检查 CSR_MIP 寄存器的状态，以决定是否有中断需要处理。
    // 如果一个挂起位被设置，并且相应的中断使能位也被设置，那么处理器会触发中断处理流程。
    // (ie & MIE_MEIE) : 处理器是否允许外部中断
    // (ip & MIP_MEIP) : 是否有外部中断挂起
    if ((ie & MIE_MEIE) && (ip & MIP_MEIP) && cpu->current_priority < PRIORITY_MACHINE_TIMER_INTERRUPT) {
        // 处理外部中断
        bool is_interrupt_occured = plic_check_interrupt(cpu->plic, (uint32_t) cpu->csr[CSR_MHARTID]);
        if (is_interrupt_occured) {
            mfprintf("CAUSE_MACHINE_EXTERNAL_INTERRUPT occur!!!\n");
            cpu->current_priority = PRIORITY_MACHINE_EXTERNAL_INTERRUPT;
            // claim 已经设置，pending已经清除，等待软件可以从claim寄存器中读取中断ID
            raise_exception(cpu, CAUSE_MACHINE_EXTERNAL_INTERRUPT);
            // 清除外部中断挂起位
            cpu->csr[CSR_MIP] &= ~MIP_MEIP;
            return true;
        }
    }

    // 检查并处理中优先级中断
    if ((ie & MIE_MTIE) && (cpu->clint->mtime >= cpu->clint->mtimecmp[cpu->csr[CSR_MHARTID]]) &&
        cpu->current_priority < PRIORITY_MACHINE_TIMER_INTERRUPT) {
        cpu->current_priority = PRIORITY_MACHINE_TIMER_INTERRUPT;
        // 处理定时器中断
        raise_exception(cpu, CAUSE_MACHINE_TIMER_INTERRUPT);
        return true;
    }

    // 检查并处理低优先级中断
    if ((ie & MIE_MSIE) && (ip & MIP_MSIP) && cpu->current_priority < PRIORITY_MACHINE_SOFTWARE_INTERRUPT) {
        cpu->current_priority = PRIORITY_MACHINE_SOFTWARE_INTERRUPT;
        // 清除软件中断挂起位
        cpu->csr[CSR_MIP] &= ~MIP_MSIP;
        // 处理软件中断
        raise_exception(cpu, CAUSE_MACHINE_SOFTWARE_INTERRUPT);
        return true;
    }
    return false;
}

