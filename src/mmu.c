#include "mmu.h"
#include "cpu.h"
#include "csr.h"

// 初始化 MMU
void init_mmu(MMU *mmu) {
    for (int i = 0; i < TLB_SIZE; i++) {
        mmu->tlb[i].valid = 0;
    }
}

// 刷新整个 TLB
void flush_tlb(MMU *mmu) {
    for (int i = 0; i < TLB_SIZE; i++) {
        mmu->tlb[i].valid = 0;
    }
}

// 刷新特定 TLB 条目
void flush_tlb_entry(MMU *mmu, uint64_t vaddr, uint64_t asid) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (mmu->tlb[i].valid && mmu->tlb[i].virtual_address == vaddr && mmu->tlb[i].asid == asid) {
            mmu->tlb[i].valid = 0;
        }
    }
}
