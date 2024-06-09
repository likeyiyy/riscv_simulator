#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "riscv_defs.h" // 包含指令定义和宏
#include "clint.h" // 包含指令定义和宏
#include "memory.h" // 包含指令定义和宏
#include "mmu.h"
#include "plic.h"
#include "uart_sim.h"

#define CLINT_BASE 0x2000000
#define CLINT_MSIP(hartid) (CLINT_BASE + 8 * (hartid))
#define CLINT_MTIME (CLINT_BASE + 0xbff8)
#define CLINT_MTIMECMP(hartid) (CLINT_BASE + 0x4000 + 8 * (hartid))



typedef struct {
    uint8_t cpu_id;         // CPU ID
    uint64_t registers[32]; // 32个通用寄存器
    uint64_t fregisters[32]; // 32个浮点寄存器
    uint64_t csr[4096]; // 4096个CSR寄存器
    uint64_t pc;            // 程序计数器
    uint8_t priv;            // 当前特权级
    uint64_t reserved_address; // 保留地址
    Memory *memory;
    MMU mmu;                 // 内存管理单元
    bool interrupt_pending;
    bool fast_mode;
    bool step_mode_refresh;
    CLINT clint;             // 核心本地中断
    PLIC plic;               // 平台级中断控制器
    int current_priority;    // 当前处理中断的优先级
    UART *uart;              // 串口
} CPU;

void cpu_init(CPU *cpu, Memory *memory, UART *uart);
void cpu_execute(CPU *cpu, Memory *memory, uint32_t instruction);
void raise_exception(CPU *cpu, uint64_t cause);
void handle_interrupt(CPU *cpu);

#endif // CPU_H
