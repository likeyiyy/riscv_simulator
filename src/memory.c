#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include "memory.h"
#include "exception.h"
#include "mfprintf.h"

MMIORegion mmio_regions[NUM_MMIO_REGIONS] = {
        { .base_addr = CLINT_BASE_ADDR, .size = CLINT_SIZE, .read = clint_read, .write = clint_write },
        { .base_addr = PLIC_BASE_ADDR, .size = PLIC_SIZE, .read = plic_read, .write = plic_write },
        { .base_addr = UART_BASE_ADDR, .size = 8, .read = uart_read, .write = uart_write },
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

void memory_free(Memory *memory) {
    if (memory->data != NULL) {
        free(memory->data);
        memory->data = NULL;
    }
    // 如果有其他需要释放的资源，可以在这里处理
}

uint32_t load_inst(Memory *memory, uint64_t address) {
    if (address >= MEMORY_END_ADDR || address < MEMORY_BASE_ADDR) {
        return 0;
    }
    uint64_t memory_addr = address - MEMORY_BASE_ADDR;
    return *((uint32_t *) (memory->data + memory_addr));
}

uint64_t memory_read(Memory *memory, uint64_t address, uint32_t size, bool is_signed) {
    if (!(address & 0x80000000)) {
        for (int i = 0; i < NUM_MMIO_REGIONS; i++) {
            MMIORegion *region = &memory->mmio_regions[i];
            if (address >= region->base_addr && address < region->base_addr + region->size) {
                return region->read(address, size);
            }
        }
    } else {
	address -= MEMORY_BASE_ADDR;
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
    return 0;
}

void memory_write(Memory *memory, uint64_t address, uint64_t value, uint32_t size) {
    if (!(address & 0x80000000)) {
        for (int i = 0; i < NUM_MMIO_REGIONS; i++) {
            MMIORegion *region = &mmio_regions[i];
            if (address >= region->base_addr && address < (region->base_addr + region->size)) {
                region->write(address, value, size);
                return;
            }
        }
    } else {
	address -= MEMORY_BASE_ADDR;
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
}
