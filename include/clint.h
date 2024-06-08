#ifndef CLINT_H
#define CLINT_H

#include <stdint.h>

typedef struct {
    uint64_t msip;   // 软件中断挂起寄存器
    uint64_t mtime;  // 定时器当前值
    uint64_t mtimecmp; // 定时器比较值
} CLINT;

void init_clint(CLINT *clint);
void set_timer_interrupt(CLINT *clint, uint64_t time);
void clear_timer_interrupt(CLINT *clint);

#endif // CLINT_H
