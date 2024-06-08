#include "plic.h"

// 初始化 PLIC
void init_plic(PLIC *plic) {
    for (int i = 0; i < PLIC_MAX_INTERRUPTS; i++) {
        plic->priority[i] = 0;
        if (i < PLIC_MAX_INTERRUPTS / 32) {
            plic->pending[i] = 0;
            plic->enable[i] = 0;
        }
    }
    plic->threshold = 0;
    plic->claim = 0;
}

// 设置中断优先级
void set_interrupt_priority(PLIC *plic, uint32_t interrupt_id, uint32_t priority) {
    if (interrupt_id < PLIC_MAX_INTERRUPTS) {
        plic->priority[interrupt_id] = priority;
    }
}

// 使能中断
void enable_interrupt(PLIC *plic, uint32_t interrupt_id) {
    if (interrupt_id < PLIC_MAX_INTERRUPTS) {
        plic->enable[interrupt_id / 32] |= (1 << (interrupt_id % 32));
    }
}

// 禁用中断
void disable_interrupt(PLIC *plic, uint32_t interrupt_id) {
    if (interrupt_id < PLIC_MAX_INTERRUPTS) {
        plic->enable[interrupt_id / 32] &= ~(1 << (interrupt_id % 32));
    }
}

// 设置中断挂起
void set_pending_interrupt(PLIC *plic, uint32_t interrupt_id) {
    if (interrupt_id < PLIC_MAX_INTERRUPTS) {
        plic->pending[interrupt_id / 32] |= (1 << (interrupt_id % 32));
    }
}

// 清除中断挂起
void clear_pending_interrupt(PLIC *plic, uint32_t interrupt_id) {
    if (interrupt_id < PLIC_MAX_INTERRUPTS) {
        plic->pending[interrupt_id / 32] &= ~(1 << (interrupt_id % 32));
    }
}

// 索取中断
uint32_t claim_interrupt(PLIC *plic) {
    for (int i = 0; i < PLIC_MAX_INTERRUPTS; i++) {
        if ((plic->pending[i / 32] & (1 << (i % 32))) &&
            (plic->enable[i / 32] & (1 << (i % 32))) &&
            (plic->priority[i] > plic->threshold)) {
            plic->claim = i;
            return i;
        }
    }
    return 0; // 无中断挂起
}

// 完成中断
void complete_interrupt(PLIC *plic, uint32_t interrupt_id) {
    clear_pending_interrupt(plic, interrupt_id);
    plic->claim = 0;
}
