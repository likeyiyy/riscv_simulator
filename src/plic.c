#include <unistd.h>
#include <printf.h>
#include <stdlib.h>
#include "plic.h"

static PLIC global_plic;

void plic_init(PLIC *plic) {
    memset(plic, 0, sizeof(PLIC));
}

PLIC *get_plic(void) {
    return &global_plic;
}



uint64_t plic_read(uint64_t address, uint32_t size) {
    PLIC *plic = get_plic();
    uint32_t offset = address - PLIC_BASE_ADDR;

    if (offset >= PLIC_PRIORITY_OFFSET && offset < PLIC_PRIORITY_OFFSET + sizeof(plic->priority)) {
        return plic->priority[(offset - PLIC_PRIORITY_OFFSET) / 4];
    } else if (offset >= PLIC_PENDING_OFFSET && offset < PLIC_PENDING_OFFSET + sizeof(plic->pending)) {
        return plic->pending[(offset - PLIC_PENDING_OFFSET) / 4];
    } else if (offset >= PLIC_ENABLE(0) && offset < PLIC_ENABLE(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_ENABLE_BASE) / sizeof(plic->enable[0]);
        uint32_t enable_offset = (offset - PLIC_ENABLE_BASE) % sizeof(plic->enable[0]);
        return plic->enable[hart_id][enable_offset / 4];
    } else if (offset >= PLIC_THRESHOLD_OFFSET(0) && offset < PLIC_THRESHOLD_OFFSET(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_THRESHOLD_BASE) / PLIC_THRESHOLD_STRIDE;
        if ((offset % PLIC_THRESHOLD_STRIDE) == 0) {
            return plic->threshold[hart_id];
        }
    } else if (offset >= PLIC_CLAIM_OFFSET(0) && offset < PLIC_CLAIM_OFFSET(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_CLAIM_BASE) / PLIC_CLAIM_STRIDE;
        if ((offset % PLIC_CLAIM_STRIDE) == 4) {
            uint32_t interrupt_id = plic->claim_complete[hart_id];
            plic->claim_complete[hart_id] = -1;
            return interrupt_id;
        }
    }
    return 0;
}

void plic_write(uint64_t address, uint64_t value, uint32_t size) {
    PLIC *plic = get_plic();
    uint32_t offset = address - PLIC_BASE_ADDR;

    if (offset >= PLIC_PRIORITY_OFFSET && offset < PLIC_PRIORITY_OFFSET + sizeof(plic->priority)) {
        plic->priority[(offset - PLIC_PRIORITY_OFFSET) / 4] = (uint32_t) value;
    } else if (offset >= PLIC_PENDING_OFFSET && offset < PLIC_PENDING_OFFSET + sizeof(plic->pending)) {
        // 挂起寄存器是只读的，不能写入
    } else if (offset >= PLIC_ENABLE(0) && offset < PLIC_ENABLE(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_ENABLE_BASE) / sizeof(plic->enable[0]);
        uint32_t enable_offset = (offset - PLIC_ENABLE_BASE) % sizeof(plic->enable[0]);
        plic->enable[hart_id][enable_offset / 4] = (uint32_t) value;
    } else if (offset >= PLIC_THRESHOLD_OFFSET(0) && offset < PLIC_THRESHOLD_OFFSET(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_THRESHOLD_BASE) / PLIC_THRESHOLD_STRIDE;
        if ((offset % PLIC_THRESHOLD_STRIDE) == 0) {
            plic->threshold[hart_id] = (uint32_t) value;
        }
    } else if (offset >= PLIC_CLAIM_OFFSET(0) && offset < PLIC_CLAIM_OFFSET(MAX_HARTS)) {
        uint32_t hart_id = (offset - PLIC_CLAIM_BASE) / PLIC_CLAIM_STRIDE;
        if ((offset % PLIC_CLAIM_STRIDE) == 4) {
            plic_complete_interrupt(hart_id, (int) value);
        }
    }
}

uint32_t plic_claim_interrupt(uint32_t hart_id) {
    PLIC *plic = get_plic();
    for (int i = 0; i < MAX_INTERRUPTS; i++) {
        if ((plic->pending[i / 32] & (1 << (i % 32))) && (plic->enable[hart_id][i / 32] & (1 << (i % 32)))) {
            if (plic->priority[i] > plic->threshold[hart_id]) {
                plic->pending[i / 32] &= ~(1 << (i % 32)); // 清除挂起状态
                plic->claim_complete[hart_id] = i;
                return i;
            }
        }
    }
    return 0; // 没有挂起的中断
}

void plic_complete_interrupt(uint32_t hart_id, int irq) {
    PLIC *plic = get_plic();
    if ((plic->enable[hart_id][irq / 32] & (1 << (irq % 32))) != 0) {
        // 确保中断源已启用
        plic->claim_complete[hart_id] = -1; // 清除 claim_complete
    }
}
