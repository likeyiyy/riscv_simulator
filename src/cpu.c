#include "cpu.h"

void cpu_init(CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        cpu->registers[i] = 0;
    }
    cpu->pc = 0;
}

void cpu_execute(CPU *cpu, uint32_t instruction) {
    // 简单的指令解码和执行（示例）
    uint32_t opcode = instruction & 0x7F;
    switch (opcode) {
        // 处理不同的操作码
        // ...
    }
}
