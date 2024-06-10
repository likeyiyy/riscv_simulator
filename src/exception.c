#include "csr.h"
#include "exception.h"

void raise_exception(CPU *cpu, uint64_t cause) {
    switch (cpu->priv) {
        case PRV_U:
            // 垂直陷入到超级模式
            cpu->csr[CSR_SEPC] = cpu->pc;
            cpu->csr[CSR_SCAUSE] = cause;
            cpu->pc = cpu->csr[CSR_STVEC];
            // save cpu->priv first
            cpu->csr[CSR_SSTATUS] = (cpu->csr[CSR_SSTATUS] & ~MSTATUS_MPP) | (cpu->priv << 11);
            // save interrupt enable first
            cpu->csr[CSR_SSTATUS] = (cpu->csr[CSR_SSTATUS] & ~SSTATUS_SPIE) | ((cpu->csr[CSR_SSTATUS] >> 5) & 0x1);
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
        uint32_t interrupt_id = claim_interrupt(&cpu->plic, cpu->csr[CSR_MHARTID]);
        if (interrupt_id != 0) {
            raise_exception(cpu, CAUSE_MACHINE_EXTERNAL_INTERRUPT);
            complete_interrupt(&cpu->plic, interrupt_id);
        }
    }

    // 检查并处理中优先级中断
    if ((ie & MIE_MTIE) && (cpu->clint.mtime >= cpu->clint.mtimecmp[cpu->csr[CSR_MHARTID]]) && cpu->current_priority < PRIORITY_MACHINE_TIMER_INTERRUPT) {
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
