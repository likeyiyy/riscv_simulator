#ifndef PLIC_H
#define PLIC_H

#include <stdint.h>

#define PLIC_MAX_INTERRUPTS 1024

typedef struct {
    uint32_t priority[PLIC_MAX_INTERRUPTS]; // 中断优先级寄存器
    uint32_t pending[PLIC_MAX_INTERRUPTS / 32]; // 中断挂起寄存器
    uint32_t enable[PLIC_MAX_INTERRUPTS / 32]; // 中断使能寄存器
    uint32_t threshold; // 中断阈值
    uint32_t claim;     // 中断索取寄存器
} PLIC;

void init_plic(PLIC *plic);
void set_interrupt_priority(PLIC *plic, uint32_t interrupt_id, uint32_t priority);
void enable_interrupt(PLIC *plic, uint32_t interrupt_id);
void disable_interrupt(PLIC *plic, uint32_t interrupt_id);
void set_pending_interrupt(PLIC *plic, uint32_t interrupt_id);
void clear_pending_interrupt(PLIC *plic, uint32_t interrupt_id);
uint32_t claim_interrupt(PLIC *plic);
void complete_interrupt(PLIC *plic, uint32_t interrupt_id);

#endif // PLIC_H
