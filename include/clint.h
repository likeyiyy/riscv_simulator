#ifndef CLINT_H
#define CLINT_H

#include <stdint.h>
#include "memory.h"

// 定义CLINT的MMIO基地址和大小
#define CLINT_BASE_ADDR 0x2000000

typedef struct {
    uint64_t msip[4096 / sizeof(uint64_t)]; // 软件中断寄存器
    uint64_t mtimecmp[4096 / sizeof(uint64_t)]; // 定时器比较寄存器
    uint64_t mtime; // 定时器寄存器
} CLINT;

CLINT* get_clint(void);
void clint_init(CLINT *clint);
uint64_t clint_read(uint64_t addr, uint32_t size);
void clint_write(uint64_t addr, uint64_t value, uint32_t size);

#endif // CLINT_H
