// clint.c

#include "clint.h"
#include "cpu.h"
#include "csr.h"
#include "mfprintf.h"
#include <string.h>
#include <unistd.h>

static CLINT global_clint;

void clint_init(CLINT *clint) {
    memset(clint, 0, sizeof(CLINT));
    // 初始化 mtimecmp 为最大值
    for (int i = 0; i < MAX_HARTS; i++) {
        clint->mtimecmp[i] = 0xFFFFFFFFFFFFFFFF;
    }
    clint->mtime = 0;
}

CLINT* get_clint(void) {
    return &global_clint;
}

uint64_t clint_read(uint64_t addr, uint32_t size) {
    CLINT *clint = get_clint();
    uint64_t offset = addr - CLINT_BASE_ADDR;

    if (offset < sizeof(clint->msip)) {
        return clint->msip[offset / sizeof(uint64_t)];
    } else if (offset >= 0x4000 && offset < 0x4000 + sizeof(clint->mtimecmp)) {
        return clint->mtimecmp[(offset - 0x4000) / sizeof(uint64_t)];
    } else if (offset == 0xBFF8) {
        return clint->mtime;
    }
    return 0;
}

void clint_write(uint64_t addr, uint64_t value, uint32_t size) {
    CLINT *clint = get_clint();
    uint64_t offset = addr - CLINT_BASE_ADDR;

    if (offset < sizeof(clint->msip)) {
        clint->msip[offset / sizeof(uint64_t)] = value;
    } else if (offset >= 0x4000 && offset < 0x4000 + sizeof(clint->mtimecmp)) {
        clint->mtimecmp[(offset - 0x4000) / sizeof(uint64_t)] = value;
    } else if (offset == 0xBFF8) {
        clint->mtime = value;
    }
}

void *timer_loop(void *arg) {
    CPU *cpu = (CPU *)arg;
    while (true) {
        // 精度：1ms
        usleep(TIMER_INTERVAL_MS * 1000); // 暂停一段时间（微秒）

        // 更新 mtime 寄存器
        cpu->clint->mtime += TIMER_INTERVAL_MS * 7000;

        // 检查是否需要触发定时器中断
        if (cpu->clint->mtime >= cpu->clint->mtimecmp[cpu->csr[CSR_MHARTID]]) {
            cpu->csr[CSR_MIP] |= MIP_MTIP; // 设置定时器中断挂起位
        }
    }
}
