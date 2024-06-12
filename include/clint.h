#ifndef CLINT_H
#define CLINT_H

#include <stdint.h>
#include "memory.h"
#include "riscv_defs.h"

// 定义CLINT的MMIO基地址和大小
#define CLINT_BASE_ADDR 0x2000000
#define CLINT_SIZE 0x10000
#define TIMER_INTERVAL_MS 10 // 定时器更新间隔（毫秒）

typedef struct {
    uint64_t msip[MAX_HARTS]; // 软件中断寄存器
    uint64_t mtimecmp[MAX_HARTS]; // 定时器比较寄存器
    uint64_t mtime; // 定时器寄存器
} CLINT;

CLINT* get_clint(void);
void clint_init(CLINT *clint);
uint64_t clint_read(uint64_t addr, uint32_t size);
void clint_write(uint64_t addr, uint64_t value, uint32_t size);
void *timer_loop(void *arg);

#endif // CLINT_H
