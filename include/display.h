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
#define REG_WIN_HEIGHT 45
#define SCREEN_WIN_START_X REG_WIN_WIDTH
#define STATUS_WIN_HEIGHT 3
#define SCREEN_WIN_WIDTH 80
#define SCREEN_WIN_HEIGHT 27

#define UART_WIN_HEIGHT 8
#define UART_WIN_WIDTH  35

#define CLINT_WIN_HEIGHT 5
#define CLINT_WIN_WIDTH  35

#define PLIC_WIN_HEIGHT 13
#define PLIC_WIN_WIDTH  45

#define SOURCE_WIN_START_X (SCREEN_WIN_START_X + SCREEN_WIN_WIDTH)
#define SOURCE_WIN_WIDTH 50
#define SOURCE_WIN_HEIGHT 34
#define STACK_WIN_START_X (SOURCE_WIN_START_X + SOURCE_WIN_WIDTH)
#define STACK_WIN_WIDTH 33
#define STACK_WIN_HEIGHT 40

typedef struct {
    CPU *cpu;
    Memory *memory;
    sem_t *sem_refresh;   // 信号量，用于通知 display 线程刷新
    WINDOW *screen_win;
    int line;
    int col;
} DisplayData;


void *update_display(void *arg);

#endif // DISPLAY_H
