#include <stdbool.h>
#include <unistd.h>  // 包含 usleep 函数的头文件

#include "csr.h"
#include "cpu.h"
#include "mmu.h"
#include "plic.h"
#include "exception.h"

// 读取 CSR 寄存器的值
uint64_t read_csr(CPU *cpu, uint32_t csr) {
    if (csr < 4096) {
        return cpu->csr[csr];
    }
    return 0;
}

// 写入 CSR 寄存器
void write_csr(CPU *cpu, uint32_t csr, uint64_t value) {
    if (csr < 4096) {
        cpu->csr[csr] = value;
    }
}

// CSRRW 指令实现
void execute_csrrw(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);
    uint32_t rs1 = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    write_csr(cpu, csr, cpu->registers[rs1]);
    cpu->registers[rd] = old_value;
}

// CSRRS 指令实现 - 含义是：CSR Read and Set
void execute_csrrs(CPU *cpu, uint32_t instruction) {
    /**
     * 1. 从指令中提取 rd, rs1, csr
     * 2. 读取 csr 寄存器的值
     * 3. 将 rs1 寄存器的值写入 csr 寄存器
     * 4. 将 csr 寄存器的值写入 rd 寄存器
     */
    uint32_t rd = RD(instruction);
    uint32_t rs1 = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    // 将 csr 寄存器的值与 rs1 寄存器的值进行按位或后的结果写入 csr 寄存器
    write_csr(cpu, csr, old_value | cpu->registers[rs1]);
    cpu->registers[rd] = old_value;
}

// CSRRC 指令实现 - 目的是：CSR Read and Clear
void execute_csrrc(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);
    uint32_t rs1 = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    // 将 csr 寄存器的值与 rs1 寄存器的值进行按位取反后的结果写入 csr 寄存器
    write_csr(cpu, csr, old_value & ~cpu->registers[rs1]);
    cpu->registers[rd] = old_value;
}

// CSRRWI 指令实现 - 目的是：CSR Read and Write Immediate
void execute_csrrwi(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);
    uint32_t imm = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    write_csr(cpu, csr, imm);
    cpu->registers[rd] = old_value;
}

// CSRRSI 指令实现 - 目的是：CSR Read and Set Immediate
void execute_csrrsi(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);
    uint32_t imm = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    write_csr(cpu, csr, old_value | imm);
    cpu->registers[rd] = old_value;
}

// CSRRCI 指令实现 - 目的是：CSR Read and Clear Immediate
void execute_csrrci(CPU *cpu, uint32_t instruction) {
    uint32_t rd = RD(instruction);
    uint32_t imm = RS1(instruction);
    uint32_t csr = (instruction >> 20) & 0xFFF;
    uint64_t old_value = read_csr(cpu, csr);
    write_csr(cpu, csr, old_value & ~imm);
    cpu->registers[rd] = old_value;
}

// MRET 指令实现 - 目的是：Machine-mode Return
void execute_mret(CPU *cpu) {
    uint64_t mstatus = read_csr(cpu, CSR_MSTATUS);
    // MRET first determiner what the new privilege mode will be according to the values of MPP
    cpu->priv = 3;

    // MPP = 0
    mstatus &= ~MSTATUS_MPP;  // 清除 MPP 字段

    // MIE = MPIE
    if ((mstatus >> 7) & 0x1) {
        mstatus |= MSTATUS_MIE;  // 恢复 MIE 位
    } else {
        mstatus &= ~MSTATUS_MIE;  // 清除 MIE 位
    }

    // MPIE = 1
    mstatus |= MSTATUS_MPIE;  // 清除 MPIE 字段
    write_csr(cpu, CSR_MSTATUS, mstatus);

    cpu->pc = cpu->csr[CSR_MEPC];
    // 重置当前处理的中断优先级
    cpu->current_priority = 0;
    cpu->trap_occurred = true;
}

// SRET 指令实现 - 目的是：Supervisor-mode Return
void execute_sret(CPU *cpu) {
    // 读取 sstatus
    uint64_t sstatus = read_csr(cpu, CSR_SSTATUS);

    // 恢复特权级别
    cpu->priv = (sstatus >> 8) & 0x1;  // 提取 SPP 字段

    // SIE = SPIE
    if ((sstatus >> 5) & 0x1) {
        sstatus |= (1 << 1);  // 恢复 SIE 位
    } else {
        sstatus &= ~(1 << 1);  // 清除 SIE 位
    }

    // SPP = 0
    sstatus &= ~SSTATUS_SPP;  // 清除 SPP 字段

    // SPIE = 1
    sstatus |= SSTATUS_SPIE;  // 清除 SPIE 字段

    // 更新 SSTATUS 寄存器
    write_csr(cpu, CSR_SSTATUS, sstatus);
    // 恢复程序计数器
    cpu->pc = cpu->csr[CSR_SEPC];
    // 重置当前处理的中断优先级
    cpu->current_priority = 0;
    cpu->trap_occurred = true;
}


// URET 指令实现 - 目的是：User-mode Return
void execute_uret(CPU *cpu) {
    cpu->pc = cpu->csr[CSR_UEPC];
    cpu->priv = (cpu->csr[CSR_USTATUS] >> 8) & 0x1;
    cpu->current_priority = 0;
}

// 假设这是模拟的内存屏障操作
void flush_write_buffers(CPU *cpu) {
    // 在实际硬件上，这可能是一个内存屏障指令
    // 在模拟器中，可以只是一个占位操作

}

// 假设这是模拟的指令缓存失效操作
void flush_instruction_cache(CPU *cpu) {
    // 在实际硬件上，这可能是一个指令缓存失效指令
    // 在模拟器中，可以只是一个占位操作

}

void execute_sfence_vma(CPU *cpu, uint32_t instruction) {
    // 通常需要刷新 TLB 或其他地址翻译缓存
    // 在模拟器中可能需要根据具体实现刷新缓存
    // 这里只是一个示例实现
    uint32_t rs1 = RS1(instruction);
    uint32_t rs2 = RS2(instruction);

    // 刷新所有地址翻译缓存
    if (rs1 == 0 && rs2 == 0) {
        flush_tlb(&cpu->mmu);
    } else {
        // 根据具体地址和 ASID 刷新
        uint64_t vaddr = cpu->registers[rs1];
        uint64_t asid = cpu->registers[rs2];
        flush_tlb_entry(&cpu->mmu, vaddr, asid);
    }
}

void execute_sinval_vma(CPU *cpu, uint32_t instruction) {
    uint32_t rs1 = RS1(instruction);
    uint32_t rs2 = RS2(instruction);

    uint64_t vaddr = cpu->registers[rs1];
    uint64_t asid = cpu->registers[rs2];
    flush_tlb_entry(&cpu->mmu, vaddr, asid);
}

void execute_sfence_w_inval(CPU *cpu, uint32_t instruction) {
    // 确保所有写操作完成
    // 在模拟器中，这可以是一个内存屏障或同步机制
    // 然后失效缓存条目
    flush_write_buffers(cpu);  // 假设存在该函数，用于确保所有写操作完成
    flush_tlb(&cpu->mmu);      // 失效整个 TLB
}

void execute_sfence_inval_ir(CPU *cpu, uint32_t instruction) {
    // 失效指令缓存
    // 确保在重新取指令之前，所有失效操作完成
    flush_instruction_cache(cpu);  // 假设存在该函数，用于失效指令缓存
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
void execute_system_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    if (funct3 >= 0x1 && funct3 <= 0x7) {
        // CSR 操作指令处理
        switch (funct3) {
            case OPCODE_CSRRW: // CSRRW
                execute_csrrw(cpu, instruction);
                break;
            case OPCODE_CSRRS: // CSRRS
                execute_csrrs(cpu, instruction);
                break;
            case OPCODE_CSRRC: // CSRRC
                execute_csrrc(cpu, instruction);
                break;
            case OPCODE_CSRRWI: // CSRRWI
                execute_csrrwi(cpu, instruction);
                break;
            case OPCODE_CSRRSI: // CSRRSI
                execute_csrrsi(cpu, instruction);
                break;
            case OPCODE_CSRRCI: // CSRRCI
                execute_csrrci(cpu, instruction);
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    } else {
        // 系统指令处理
        uint32_t imm = instruction >> 20;
        switch (imm) {
            case OPCODE_ECALL:
                execute_ecall(cpu);
                break;
            case OPCODE_EBREAK:
                raise_exception(cpu, CAUSE_BREAKPOINT);
                break;
            case OPCODE_SRET:
                // SRET
                execute_sret(cpu);
                break;
            case OPCODE_MRET:
                // MRET
                execute_mret(cpu);
                break;
            case OPCODE_URET:
                // URET
                execute_uret(cpu);
                break;
            case OPCODE_WFI:
                // WFI (Wait For Interrupt)
                // 模拟处理器等待中断
                // 在实际的模拟器中，这里可能需要实现一个事件循环或等待条件变量。
                // 在这里，我们使用一个简单的循环来模拟等待中断。
                while (true) {
                    // 先检查并处理中断
                    bool result = handle_interrupt(cpu);
                    if (result) {
                        break;
                    }
                    usleep(1000);
                }
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    }
}
