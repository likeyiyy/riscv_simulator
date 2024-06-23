#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "mmio.h"

#define MEMORY_BASE_ADDR 0x80000000
#define MEMORY_SIZE  (128 * 1024 * 1024)
#define MEMORY_END_ADDR (MEMORY_BASE_ADDR + MEMORY_SIZE)


typedef struct {
    uint8_t *data;
    MMIORegion *mmio_regions;
} Memory;

void memory_init(Memory *memory);
void memory_free(Memory *memory);
uint32_t load_inst(Memory *memory, uint64_t address);
void memory_write(Memory *memory, uint64_t address, uint64_t value, uint32_t size);
uint64_t memory_read(Memory *memory, uint64_t address, uint32_t size, bool is_signed);


#endif // MEMORY_H
