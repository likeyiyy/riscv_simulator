#ifndef RISCV_SIMULATOR_H
#define RISCV_SIMULATOR_H

#include "keyboard.h"
#include "display.h"
#include "cpu.h"
#include "memory.h"
#include <semaphore.h>

typedef struct {
    CPU *cpu;
    Memory *memory;
    DisplayData *display;
    KeyBoardData* keyboard;
    sem_t *sem_continue;  // 信号量，用于通知 CPU 线程继续执行
    sem_t *sem_refresh;   // 信号量，用于通知 display 线程刷新
    const char *input_file; // 输入文件
    uint64_t load_address; // 开始地址
    uint64_t end_address; // 结束地址
} Simulator;

void* cpu_simulator(void *arg);

#endif // RISCV_SIMULATOR_H
