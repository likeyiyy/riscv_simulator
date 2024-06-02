#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef struct {
    uint32_t registers[32]; // 32个通用寄存器
    uint32_t pc;            // 程序计数器
} CPU;

void cpu_init(CPU *cpu);
void cpu_execute(CPU *cpu, uint32_t instruction);

#endif // CPU_H
