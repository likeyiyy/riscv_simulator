#include <stdio.h>
#include "cpu.h"
#include "memory.h"

int main() {
    CPU cpu;
    Memory memory;

    cpu_init(&cpu);
    memory_init(&memory);

    // 加载指令到内存
    uint32_t instruction = 0x00000013; // NOP指令（示例）
    memory_store_word(&memory, 0, instruction);

    // 模拟指令执行
    while (cpu.pc < MEMORY_SIZE) {
        uint32_t instruction = memory_load_word(&memory, cpu.pc);
        cpu_execute(&cpu, instruction);
        cpu.pc += 4; // 下一条指令
    }

    printf("Simulation complete.\n");

    return 0;
}
