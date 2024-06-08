#include "csr.h"
#include "cpu.h"

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
    cpu->pc = cpu->csr[CSR_MEPC];
    cpu->priv = (cpu->csr[CSR_MSTATUS] >> 11) & 0x3;
}

// SRET 指令实现 - 目的是：Supervisor-mode Return
void execute_sret(CPU *cpu) {
    cpu->pc = cpu->csr[CSR_SEPC];
    cpu->priv = (cpu->csr[CSR_SSTATUS] >> 8) & 0x1;
}

// URET 指令实现 - 目的是：User-mode Return
void execute_uret(CPU *cpu) {
    cpu->pc = cpu->csr[CSR_UEPC];
    cpu->priv = (cpu->csr[CSR_USTATUS] >> 8) & 0x1;
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
                raise_exception(cpu, CAUSE_USER_ECALL + cpu->priv);
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
                while (!cpu->interrupt_pending) {
                    // 等待中断发生
                    // 在真实的实现中，可能需要处理事件或检查外部输入。
                }
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    }
}
