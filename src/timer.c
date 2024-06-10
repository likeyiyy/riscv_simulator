#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "csr.h"
#include "clint.h"
#include "cpu.h"

#define TIMER_INTERVAL_MS 10 // 定时器更新间隔（毫秒）

void *timer_thread(void *arg) {
    CPU *cpu = (CPU *)arg;
    while (true) {
        usleep(TIMER_INTERVAL_MS * 1000); // 暂停一段时间（微秒）

        // 更新 mtime 寄存器
        cpu->clint->mtime += TIMER_INTERVAL_MS;

        // 检查是否需要触发定时器中断
        if (cpu->clint->mtime >= cpu->clint->mtimecmp[cpu->csr[CSR_MHARTID]]) {
            cpu->csr[CSR_MIP] |= MIP_MTIP; // 设置定时器中断挂起位
        }
    }
}
