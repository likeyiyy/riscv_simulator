#ifndef PLIC_H
#define PLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// 定义PLIC的MMIO基地址和大小
#define PLIC_BASE_ADDR 0x0C000000
#define PLIC_SIZE 0x400000

// 定义PLIC寄存器的偏移
#define PLIC_PRIORITY_OFFSET 0x000000
#define PLIC_PENDING_OFFSET  0x001000
#define PLIC_ENABLE_OFFSET   0x002000
#define PLIC_THRESHOLD_OFFSET 0x200000
#define PLIC_CLAIM_OFFSET   0x200004

#define MAX_INTERRUPTS 1024

// PLIC数据结构
typedef struct {
    uint32_t priority[MAX_INTERRUPTS];
    uint32_t pending[MAX_INTERRUPTS / 32];
    uint32_t enable[MAX_INTERRUPTS / 32];
    uint32_t threshold;
    uint32_t claim_complete;
} PLIC;

// 初始化PLIC
void plic_init(void);

// 读取PLIC寄存器
uint32_t plic_read(uint32_t address);

// 写入PLIC寄存器
void plic_write(uint32_t address, uint32_t value);

// 触发中断
void trigger_interrupt(PLIC *plic, int interrupt_id);

// 声明中断
int claim_interrupt(PLIC *plic, int cpu_id);

// 完成中断
void complete_interrupt(PLIC *plic, int interrupt_id);

// 模拟外部中断生成
void* external_interrupt_simulator(void* arg);

#endif // PLIC_H
