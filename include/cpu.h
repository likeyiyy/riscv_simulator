#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "riscv_defs.h" // 包含指令定义和宏
#include "memory.h" // 包含指令定义和宏

typedef struct {
    uint64_t registers[32]; // 32个通用寄存器
    uint64_t pc;            // 程序计数器
} CPU;

void cpu_init(CPU *cpu);
void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction);

#endif // CPU_H
