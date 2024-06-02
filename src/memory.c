#include "memory.h"

void memory_init(Memory *memory) {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        memory->data[i] = 0;
    }
}

uint32_t memory_load_word(Memory *memory, uint32_t address) {
    if (address >= MEMORY_SIZE) {
        return 0;
    }
    return *((uint32_t *)(memory->data + address));
}

void memory_store_word(Memory *memory, uint32_t address, uint32_t value) {
    if (address < MEMORY_SIZE) {
        *((uint32_t *)(memory->data + address)) = value;
    }
}
