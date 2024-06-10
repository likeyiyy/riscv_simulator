// clint.c

#include "clint.h"
#include <string.h>

static CLINT global_clint;

void clint_init(CLINT *clint) {
    memset(clint, 0, sizeof(CLINT));
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
