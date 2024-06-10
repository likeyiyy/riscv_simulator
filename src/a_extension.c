#include "a_extension.h"
#include "csr.h"
#include "exception.h"

inline void execute_lr_w(CPU *cpu, uint32_t rd, uint32_t rs1) {
    uint64_t address = cpu->registers[rs1];
    uint64_t value = memory_read(cpu->memory, address, 8, false);
    cpu->registers[rd] = value;
    cpu->reserved_address = address;
}

inline void execute_sc_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t value = cpu->registers[rs2];
    if (cpu->reserved_address == address) {
        memory_write(cpu->memory, address, value, 8);
        cpu->reserved_address = 0;
        cpu->registers[rd] = 0;
    } else {
        cpu->registers[rd] = 1;
    }
}

inline void execute_amoswap_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    memory_write(cpu->memory, address, src_value, 8);
    cpu->registers[rd] = mem_value;
}

inline void execute_amoadd_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行加法操作
    uint64_t result = mem_value + src_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}

inline void execute_amoand_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行按位与操作
    uint64_t result = mem_value & src_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}

inline void execute_amoxor_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行按位异或操作
    uint64_t result = mem_value ^ src_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}


inline void execute_amoor_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行按位或操作
    uint64_t result = mem_value | src_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}

inline void execute_amomin_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    int64_t src_value = (int64_t)cpu->registers[rs2];
    // 从内存地址加载值
    int64_t mem_value = (int64_t)memory_read(cpu->memory, address, 8, true);
    // 执行有符号比较并选择较小的值
    int64_t result = (src_value < mem_value) ? src_value : mem_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, (uint64_t)result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = (uint64_t)mem_value;
}


inline void execute_amomax_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    int64_t src_value = (int64_t)cpu->registers[rs2];
    // 从内存地址加载值
    int64_t mem_value = (int64_t)memory_read(cpu->memory, address, 8, true);
    // 执行有符号比较并选择较大的值
    int64_t result = (src_value > mem_value) ? src_value : mem_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, (uint64_t)result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = (uint64_t)mem_value;
}

inline void execute_amominu_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行无符号比较并选择较小的值
    uint64_t result = (src_value < mem_value) ? src_value : mem_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}

inline void execute_amomaxu_w(CPU *cpu, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint64_t address = cpu->registers[rs1];
    uint64_t src_value = cpu->registers[rs2];
    // 从内存地址加载值
    uint64_t mem_value = memory_read(cpu->memory, address, 8, false);
    // 执行无符号比较并选择较大的值
    uint64_t result = (src_value > mem_value) ? src_value : mem_value;
    // 将结果存储回内存地址
    memory_write(cpu->memory, address, result, 8);
    // 将内存中的旧值返回到目标寄存器
    cpu->registers[rd] = mem_value;
}



void execute_atomic_instruction(CPU *cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t funct7 = (instruction >> 25) & 0x7F;
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;

    if (funct3 == FUNCT3_LR && funct7 == 0x0) {
        execute_lr_w(cpu, rd, rs1);
    } else if (funct3 == FUNCT3_SC && funct7 == 0x0) {
        execute_sc_w(cpu, rd, rs1, rs2);
    } else {
        switch (funct7) {
            case FUNCT7_AMOSWAP:
                execute_amoswap_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOADD:
                execute_amoadd_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOXOR:
                execute_amoxor_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOAND:
                execute_amoand_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOOR:
                execute_amoor_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOMIN:
                execute_amomin_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOMAX:
                execute_amomax_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOMINU:
                execute_amominu_w(cpu, rd, rs1, rs2);
                break;
            case FUNCT7_AMOMAXU:
                execute_amomaxu_w(cpu, rd, rs1, rs2);
                break;
            default:
                raise_exception(cpu, CAUSE_ILLEGAL_INSTRUCTION);
                break;
        }
    }
}
