#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "riscv_defs.h" // 包含指令定义和宏
#include "memory.h" // 包含指令定义和宏

typedef struct {
    uint64_t registers[32]; // 32个通用寄存器
    uint64_t fregisters[32]; // 32个浮点寄存器
    uint64_t csr[4096]; // 4096个CSR寄存器
    uint64_t pc;            // 程序计数器
    uint8_t priv;            // 当前特权级
    uint64_t reserved_address; // 保留地址
    Memory *memory;
} CPU;

void cpu_init(CPU *cpu, Memory *memory);
void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction);

#endif // CPU_H
