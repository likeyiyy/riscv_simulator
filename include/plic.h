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
#define PLIC_ENABLE_BASE   0x002000
#define PLIC_THRESHOLD_BASE 0x200000
#define PLIC_THRESHOLD_STRIDE 0x1000
#define PLIC_CLAIM_BASE   0x200004
#define PLIC_CLAIM_STRIDE 0x1000

#define MAX_INTERRUPTS 128
#define MAX_HARTS 8 // 假设最多有8个hart

#define PLIC_ENABLE(hart) (PLIC_ENABLE_BASE + (hart) * 0x80)
#define PLIC_THRESHOLD_OFFSET(hart) (PLIC_THRESHOLD_BASE + (hart) * 0x1000)
#define PLIC_CLAIM_OFFSET(hart) (PLIC_CLAIM_BASE + (hart) * 0x1000)

#define UART0_IRQ 10
// PLIC数据结构
typedef struct {
    // 每个中断源都有0-7的优先级，0表示禁用，7表示最高优先级，会和阈值寄存器比较
    uint32_t priority[MAX_INTERRUPTS];
    // pending 是关于中断挂起的位图，每个中断源有一个位，1表示挂起，0表示未挂起，所以共需要128位，即128/32=4个32位的uint32_t
    uint32_t pending[MAX_INTERRUPTS / 32];

    uint32_t threshold[MAX_HARTS]; // 每个hart有一个阈值寄存器，用于比较优先级

    uint32_t enable[MAX_HARTS][MAX_INTERRUPTS / 32]; // 每个hart有一个128位的enable位图

    uint32_t claim_complete[MAX_HARTS]; // 每个hart有一个claim/complete寄存器
} PLIC;


// 初始化PLIC
void plic_init(PLIC *plic);

PLIC* get_plic(void);

// 读取PLIC寄存器
uint64_t plic_read(uint64_t address, uint32_t size);

// 写入PLIC寄存器
void plic_write(uint64_t address, uint64_t value, uint32_t size);
uint32_t plic_claim_interrupt(uint32_t hart_id);
void plic_complete_interrupt(uint32_t hart_id, int irq);



#endif // PLIC_H
