#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <semaphore.h>
#include "cpu.h"
#include "memory.h"

#define STACK_SIZE 32
typedef struct {
    CPU *cpu;
    Memory *memory;
    uint32_t pc;
    sem_t *sem;
} DisplayData;


void *update_display(void *arg);

#endif // DISPLAY_H
