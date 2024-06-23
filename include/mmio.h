#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>
#include <stddef.h>
#include "plic.h"
#include "clint.h"
#include "uart.h"

#define NUM_MMIO_REGIONS 3

typedef struct {
    uint64_t base_addr;
    uint64_t size;
    uint64_t (*read)(uint64_t address, uint32_t size);
    void (*write)(uint64_t address, uint64_t value, uint32_t size);
} MMIORegion;

#endif // MMIO_H
