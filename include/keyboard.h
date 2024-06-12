#ifndef RISCV_KEYBOARD_H
#define RISCV_KEYBOARD_H

#include <semaphore.h>
#include "cpu.h"

// 定义模式
typedef enum {
    NONE_MODE,
    CPU_MODE,
    UART_MODE
} Mode;

typedef struct {
    CPU *cpu;
    char key;  // 键值
    sem_t *sem_continue;  // 信号量，用于通知 CPU 线程继续执行
    sem_t *sem_refresh;   // 信号量，用于通知 display 线程刷新
} KeyBoardData;

void switch_mode(Mode new_mode);
Mode get_mode(void);
void* keyboard_input(void *arg);


#endif // RISCV_KEYBOARD_H
