#include "mmu.h"

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

// 地址翻译（简化示例，不包含完整的页表遍历逻辑）
uint64_t translate_address(CPU *cpu, uint64_t virtual_address) {
    MMU *mmu = &cpu->mmu;
    uint64_t satp = cpu->csr[CSR_SATP];
    uint64_t root_page_table = satp & PAGE_NUMBER_MASK;
    uint64_t page_offset = virtual_address & PAGE_OFFSET_MASK;

    // 检查 TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        if (mmu->tlb[i].valid && mmu->tlb[i].virtual_address == virtual_address) {
            return mmu->tlb[i].physical_address | page_offset;
        }
    }

    // TLB 未命中，进行页表遍历
    for (int level = 2; level >= 0; level--) {
        uint64_t vpn = (virtual_address >> (12 + level * 9)) & 0x1FF;
        uint64_t pte_address = root_page_table + vpn * sizeof(uint64_t);
        uint64_t pte = memory_load_word(cpu->memory, pte_address);

        if (!(pte & PTE_V)) {
            raise_exception(cpu, CAUSE_PAGE_FAULT);
            return 0;
        }

        if (level == 0) {
            if (!(pte & (PTE_R | PTE_X))) {
                raise_exception(cpu, CAUSE_PAGE_FAULT);
                return 0;
            }
            uint64_t physical_address = (pte & PAGE_NUMBER_MASK) | page_offset;

            // 更新 TLB
            int tlb_index = virtual_address % TLB_SIZE;
            mmu->tlb[tlb_index].virtual_address = virtual_address;
            mmu->tlb[tlb_index].physical_address = physical_address;
            mmu->tlb[tlb_index].asid = satp >> 44;  // ASID 通常在 satp 的高位
            mmu->tlb[tlb_index].valid = 1;

            return physical_address;
        }

        root_page_table = pte & PAGE_NUMBER_MASK;
    }

    raise_exception(cpu, CAUSE_PAGE_FAULT);
    return 0;
}
