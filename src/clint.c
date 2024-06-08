#include "cpu.h"
#include "clint.h"

// 在 RISC-V 架构中，CLINT（Core Local Interruptor）是一个包含定时器和软件中断功能的模块。
// 初始化 CLINT
void init_clint(CLINT *clint, uint8_t cpu_id, Memory *memory) {
    // 设置 CLINT 寄存器的指针
    clint->msip = (uint64_t *)(memory->data + CLINT_MSIP(cpu_id));
    clint->mtime = (uint64_t *)(memory->data + CLINT_MTIME);
    *clint->mtime = 0;
    clint->mtimecmp = (uint64_t *)(memory->data + CLINT_MTIMECMP(cpu_id));
    *clint->mtimecmp = 0xFFFFFFFFFFFFFFFF;
}
