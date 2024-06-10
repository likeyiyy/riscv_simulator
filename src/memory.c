#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include "memory.h"
#include "exception.h"

MMIORegion mmio_regions[NUM_MMIO_REGIONS] = {
        { .base_addr = CLINT_BASE_ADDR, .size = 0x1000, .read = clint_read, .write = clint_write },
        { .base_addr = PLIC_BASE_ADDR, .size = 0x1000, .read = clint_read, .write = clint_write },
        { .base_addr = UART_BASE_ADDR, .size = 0x1000, .read = uart_read, .write = uart_write },
        // 添加其他 MMIO 区域
};

void memory_init(Memory *memory) {
    memory->data = (uint8_t *) malloc(MEMORY_SIZE);
    if (memory->data == NULL) {
        fprintf(stderr, "Failed to allocate memory data\n");
        free(memory);
        exit(1);
    }
    memory->mmio_regions = mmio_regions;
}


uint64_t memory_read(Memory *memory, uint64_t address, uint32_t size, bool is_signed) {
    for (size_t i = 0; i < NUM_MMIO_REGIONS; i++) {
        MMIORegion *region = &memory->mmio_regions[i];
        if (address >= region->base_addr && address < region->base_addr + region->size) {
            return region->read(address, size);
        }
    }
    switch (size) {
        case 1:
            return is_signed ? (int8_t)memory->data[address] : memory->data[address];
        case 2:
            return is_signed ? (int16_t)*(uint16_t *)&memory->data[address] : *(uint16_t *)&memory->data[address];
        case 4:
            return is_signed ? (int32_t)*(uint32_t *)&memory->data[address] : *(uint32_t *)&memory->data[address];
        case 8:
            return is_signed ? (int64_t)*(uint64_t *)&memory->data[address] : *(uint64_t *)&memory->data[address];
        default:
            return 0;
    }
}

void memory_write(Memory *memory, uint64_t address, uint64_t value, uint32_t size) {
    for (int i = 0; i < NUM_MMIO_REGIONS; i++) {
        MMIORegion *region = &mmio_regions[i];
        if (address >= region->base_addr && address < (region->base_addr + region->size)) {
            region->write(address - region->base_addr, value);
            return;
        }
    }
    switch (size) {
        case 1:
            memory->data[address] = value & 0xFF;
            break;
        case 2:
            *(uint16_t *)&memory->data[address] = value & 0xFFFF;
            break;
        case 4:
            *(uint32_t *)&memory->data[address] = value & 0xFFFFFFFF;
            break;
        case 8:
            *(uint64_t *)&memory->data[address] = value;
            break;
        default:
            // impossible
            break;
    }
}
