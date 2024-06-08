//
// Created by gllue new on 2024/6/8.
//
#include <stdatomic.h>
#include "fence_inst.h"


void execute_fence(CPU *cpu, uint32_t instruction) {
    // FENCE 指令不需要实际的操作，在模拟器中可以作为屏障
    atomic_thread_fence(memory_order_seq_cst);
}

void execute_fence_i(CPU *cpu, uint32_t instruction) {
    // FENCE.I 指令在模拟器中通常不需要实际操作
    // 在实际硬件上，这会刷新指令缓存
}

void execute_fence_tso(CPU *cpu, uint32_t instruction) {
    atomic_thread_fence(memory_order_seq_cst);
}

void execute_misc_mem_instructions(CPU *cpu, uint32_t instruction) {
    if ((instruction & 0x07000000) == 0x00000000) {
        execute_fence(cpu, instruction);
    } else if ((instruction & 0x07000000) == 0x01000000) {
        execute_fence_i(cpu, instruction);
    } else if ((instruction & 0xF00FFFFF) == 0x8000000F) {
        execute_fence_tso(cpu, instruction);
    }
}
