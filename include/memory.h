#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE  (128 * 1024 * 1024)// 128MB内存

typedef struct {
    uint8_t data[MEMORY_SIZE];
} Memory;

void memory_init(Memory *memory);
uint32_t memory_load_word(Memory *memory, uint64_t address);
uint64_t memory_load_dword(Memory *memory, uint64_t address);
void memory_store_word(Memory *memory, uint64_t address, uint32_t value);

#endif // MEMORY_H
