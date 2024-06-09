#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <semaphore.h>
#include "cpu.h"
#include "memory.h"
#include "uart_sim.h"

#define STACK_SIZE 32

typedef struct {
    CPU *cpu;
    Memory *memory;
    uint32_t pc;
    sem_t *sem_refresh;   // 信号量，用于通知 display 线程刷新
    sem_t *sem_continue;  // 信号量，用于通知 CPU 线程继续执行
    UART *uart; // 新增：UART结构体指针
} DisplayData;


void *update_display(void *arg);

#endif // DISPLAY_H
