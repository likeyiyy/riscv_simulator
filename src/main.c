#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    CPU cpu;
    Memory memory;

    cpu_init(&cpu);
    memory_init(&memory);

    // 打开包含指令的文件
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // 从文件中读取指令并加载到内存
    uint32_t address = 0;
    uint32_t instruction;
    while (fread(&instruction, sizeof(uint32_t), 1, file)) {
        memory_store_word(&memory, address, instruction);
        address += 4;
    }

    fclose(file);

    // 初始化寄存器
    cpu.registers[2] = 5;  // x2 = 5
    cpu.registers[3] = 10; // x3 = 10

    // 模拟指令执行
    while (cpu.pc < MEMORY_SIZE) {
        uint32_t instruction = memory_load_word(&memory, cpu.pc);
        // 判断指令是否全为0
        if (instruction == 0) {
            printf("All instructions are zero, exiting.\n");
            break;
        }
        cpu_execute(&cpu, instruction);
        cpu.pc += 4; // 下一条指令
    }

    printf("x1 = %llu\n", cpu.registers[1]); // 应输出15

    return 0;
}
