#include "clint.h"

// 初始化 CLINT
void init_clint(CLINT *clint) {
    clint->msip = 0;
    clint->mtime = 0;
    clint->mtimecmp = 0xFFFFFFFFFFFFFFFF; // 初始值设置为最大
}

// 设置定时器中断
void set_timer_interrupt(CLINT *clint, uint64_t time) {
    clint->mtimecmp = time;
}

// 清除定时器中断
void clear_timer_interrupt(CLINT *clint) {
    clint->mtimecmp = 0xFFFFFFFFFFFFFFFF; // 重新设置为最大值
}
