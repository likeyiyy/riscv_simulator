#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <semaphore.h>
#include "cpu.h"
#include "memory.h"
#include "uart.h"

#define STACK_SIZE 32
#define REG_WIN_START_X 0
#define REG_WIN_WIDTH 30
#define REG_WIN_HEIGHT 40
#define SCREEN_WIN_START_X REG_WIN_WIDTH
#define SCREEN_WIN_WIDTH 80
#define SCREEN_WIN_HEIGHT 27
#define SOURCE_WIN_START_X (SCREEN_WIN_START_X + SCREEN_WIN_WIDTH)
#define SOURCE_WIN_WIDTH 50
#define SOURCE_WIN_HEIGHT 34
#define STACK_WIN_START_X (SOURCE_WIN_START_X + SOURCE_WIN_WIDTH)
#define STACK_WIN_WIDTH 33
#define STACK_WIN_HEIGHT 40

typedef struct {
    CPU *cpu;
    Memory *memory;
    uint32_t pc;
    sem_t *sem_refresh;   // 信号量，用于通知 display 线程刷新
} DisplayData;


void *update_display(void *arg);

#endif // DISPLAY_H
