#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

void memory_init(Memory *memory) {
    memory->data = (uint8_t *) malloc(MEMORY_SIZE);
    if (memory->data == NULL) {
        fprintf(stderr, "Failed to allocate memory data\n");
        free(memory);
        exit(1);
    }

}

uint32_t memory_load_word(Memory *memory, uint64_t address) {
    if (address >= MEMORY_SIZE) {
        return 0;
    }
    return *((uint32_t *) (memory->data + address));
}

uint64_t memory_load_dword(Memory *memory, uint64_t address) {
    if (address >= MEMORY_SIZE) {
        return 0;
    }
    return *((uint64_t *) (memory->data + address));
}

void memory_store_word(Memory *memory, uint64_t address, uint32_t value) {
    if (address < MEMORY_SIZE) {
        *((uint32_t *) (memory->data + address)) = value;
    }
}
