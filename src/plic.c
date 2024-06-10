#include <unistd.h>
#include <printf.h>
#include <stdlib.h>
#include "plic.h"

static PLIC global_plic;

void plic_init(PLIC *plic) {
    memset(plic, 0, sizeof(PLIC));
}

PLIC* get_plic(void) {
    return &global_plic;
}

uint64_t plic_read(uint64_t address, uint32_t size) {
    PLIC *plic = get_plic();
    uint32_t offset = address - PLIC_BASE_ADDR;
    if (offset >= PLIC_PRIORITY_OFFSET && offset < PLIC_PRIORITY_OFFSET + sizeof(plic->priority)) {
        return plic->priority[(offset - PLIC_PRIORITY_OFFSET) / 4];
    } else if (offset >= PLIC_PENDING_OFFSET && offset < PLIC_PENDING_OFFSET + sizeof(plic->pending)) {
        return plic->pending[(offset - PLIC_PENDING_OFFSET) / 4];
    } else if (offset >= PLIC_ENABLE_OFFSET && offset < PLIC_ENABLE_OFFSET + sizeof(plic->enable)) {
        return plic->enable[(offset - PLIC_ENABLE_OFFSET) / 4];
    } else if (offset == PLIC_THRESHOLD_OFFSET) {
        return plic->threshold;
    } else if (offset == PLIC_CLAIM_OFFSET) {
        int interrupt_id = plic->claim_complete;
        plic->claim_complete = -1;
        return interrupt_id;
    } else {
        // 无效地址，返回0
        return 0;
    }
}

void plic_write(uint64_t address, uint64_t value, uint32_t size) {
    PLIC *plic = get_plic();
    uint32_t offset = address - PLIC_BASE_ADDR;
    if (offset >= PLIC_PRIORITY_OFFSET && offset < PLIC_PRIORITY_OFFSET + sizeof(plic->priority)) {
        plic->priority[(offset - PLIC_PRIORITY_OFFSET) / 4] = value;
    } else if (offset >= PLIC_PENDING_OFFSET && offset < PLIC_PENDING_OFFSET + sizeof(plic->pending)) {
        // 挂起寄存器是只读的，不能写入
    } else if (offset >= PLIC_ENABLE_OFFSET && offset < PLIC_ENABLE_OFFSET + sizeof(plic->enable)) {
        plic->enable[(offset - PLIC_ENABLE_OFFSET) / 4] = value;
    } else if (offset == PLIC_THRESHOLD_OFFSET) {
        plic->threshold = value;
    } else if (offset == PLIC_CLAIM_OFFSET) {
        plic->claim_complete = value;
    }
}

void trigger_interrupt(PLIC *plic, int interrupt_id) {
    plic->pending[interrupt_id / 32] |= (1 << (interrupt_id % 32));
}

int claim_interrupt(PLIC *plic, int cpu_id) {
    for (int i = 0; i < MAX_INTERRUPTS; i++) {
        if ((plic->pending[i / 32] & (1 << (i % 32))) && (plic->enable[i / 32] & (1 << (i % 32)))) {
            plic->pending[i / 32] &= ~(1 << (i % 32)); // 清除挂起状态
            plic->claim_complete = i;
            return i;
        }
    }
    return -1; // 没有挂起的中断
}

void complete_interrupt(PLIC *plic, int interrupt_id) {
    // 中断处理完成后的操作
    plic->claim_complete = -1;
}
