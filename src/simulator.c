#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // for usleep
#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include "simulator.h"
#include "exception.h"
#include "csr.h"

// 获取当前的 TSC 值
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
            "rdtsc"
            : "=a" (lo), "=d" (hi)
            );
    return ((uint64_t)hi << 32) | lo;
}

void* cpu_simulator(void *arg) {
    Simulator *simulator = (Simulator *)arg;
    CPU *cpu = simulator->cpu;
    KeyBoardData* keyboard_data = simulator->keyboard;
    Memory *memory = cpu->memory;

    // Simulate instruction execution
    char ch;
    uint32_t instruction;
    struct timeval start, end;
    long seconds, useconds;
    double elapsed;
    // 获取开始时的 TSC 值
    uint64_t start_tsc;

    while (1) {
        if (cpu->pc < 0x100 || cpu->pc >= MEMORY_SIZE) {
            raise_exception(cpu, CAUSE_LOAD_ACCESS_FAULT);
        }
        instruction = load_inst(memory, cpu->pc);

        if (!cpu->fast_mode) {
            sem_wait(simulator->sem_continue); // Wait for display thread to finish updating
            ch = keyboard_data->key; // Wait for user input in step mode
            if (ch == 'q') break; // Quit the program
            if (ch == 's') {
                cpu_execute(cpu, instruction);
                cpu->csr[CSR_MINSTRET] += 1;
                sem_post(simulator->sem_refresh); // Notify display thread to refresh
            }
            if (ch == 'c') {
                cpu->fast_mode = true;  // Fast mode
                cpu_execute(cpu, instruction);
                cpu->csr[CSR_MINSTRET] += 1;
                sem_post(simulator->sem_refresh);

                start_tsc = rdtsc();
                gettimeofday(&start, NULL);
            }

        } else {
            Mode mode = get_mode();
            if (mode == CPU_MODE) {
                ch = keyboard_data->key;
                if (ch == 's') {
                    cpu->fast_mode = false;
                    sem_post(simulator->sem_refresh);
                    continue;
                }
            }
            if (cpu->pc == simulator->end_address) {
                cpu->fast_mode = false;
                sem_post(simulator->sem_refresh);
                // 获取结束时间
                gettimeofday(&end, NULL);

                // 计算执行时间
                seconds = end.tv_sec - start.tv_sec;
                useconds = end.tv_usec - start.tv_usec;
                elapsed = seconds + useconds / 1000000.0;
                // 获取结束时的 TSC 值
                uint64_t end_tsc = rdtsc();
                // 计算执行的时钟周期数
                uint64_t cycles = end_tsc - start_tsc;
                mvprintw(40, 1, "Elapsed CPU cycles: %llu\n", cycles);

                mvprintw(41, 1, " %.6fs\n", elapsed);
            } else {
                cpu_execute(cpu, instruction);
                cpu->csr[CSR_MINSTRET] += 1;
            }
        }
    }
    return NULL;
}
