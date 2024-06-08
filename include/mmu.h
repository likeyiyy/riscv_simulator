//
// Created by gllue new on 2024/6/8.
//

#ifndef RISCSIMULATOR_MMU_H
#define RISCSIMULATOR_MMU_H
#include <stdint.h>
#include "cpu.h"

#define TLB_SIZE 16  // 示例 TLB 大小，可以根据需要调整

typedef struct {
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t asid;  // Address Space Identifier
    int valid;      // TLB 条目是否有效
} TLBEntry;

typedef struct {
    TLBEntry tlb[TLB_SIZE];
} MMU;

void init_mmu(MMU *mmu);
void flush_tlb(MMU *mmu);
void flush_tlb_entry(MMU *mmu, uint64_t vaddr, uint64_t asid);
uint64_t translate_address(CPU *cpu, uint64_t virtual_address);
#endif //RISCSIMULATOR_MMU_H
