#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int main() {
    CPU cpu;
    Memory memory;

    cpu_init(&cpu);
    memory_init(&memory);

    // 加载ADD指令到内存：add x1, x2, x3
    uint32_t add_instruction = 0x003100B3; // R-type指令示例
    memory_store_word(&memory, 0, add_instruction);

    // 初始化寄存器
    cpu.registers[2] = 5;  // x2 = 5
    cpu.registers[3] = 10; // x3 = 10

    // 模拟指令执行
    while (cpu.pc < MEMORY_SIZE) {
        uint32_t instruction = memory_load_word(&memory, cpu.pc);
        cpu_execute(&cpu, instruction);
        cpu.pc += 4; // 下一条指令
    }

    printf("x1 = %lu\n", cpu.registers[1]); // 应输出15

    return 0;
}