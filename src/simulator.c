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

void load_file_to_memory(const char *filename, Memory *memory, size_t address) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 确保内存大小足够，并且检查地址的合法性
    if (file_size + address > MEMORY_END_ADDR) {
        fprintf(stderr, "File size exceeds memory size or invalid address\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    address -= MEMORY_BASE_ADDR;

    // 读取文件内容到内存
    size_t bytes_read = fread(memory->data + address, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Failed to read complete file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}


void reset_system(Simulator *simulator) {
    clint_init(simulator->cpu->clint);
    plic_init(simulator->cpu->plic);
    uart_init(simulator->cpu->uart); // 初始化 UART
    memory_free(simulator->cpu->memory);
    memory_init(simulator->cpu->memory);
    cpu_init(simulator->cpu, simulator->cpu->memory, simulator->cpu->clint, simulator->cpu->plic, simulator->cpu->uart);

    load_file_to_memory(simulator->input_file, simulator->cpu->memory, simulator->load_address);
    simulator->cpu->pc = simulator->load_address;

    simulator->display->line = 1;
    simulator->display->col = 1;
    wclear(simulator->display->screen_win);
    box(simulator->display->screen_win, 0, 0);
    wrefresh(simulator->display->screen_win);
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
        if (cpu->pc < 0x100 || cpu->pc >= MEMORY_END_ADDR) {
            raise_exception(cpu, CAUSE_LOAD_ACCESS_FAULT);
        }
        instruction = load_inst(memory, cpu->pc);

        if (!cpu->fast_mode) {
            sem_wait(simulator->sem_continue); // Wait for display thread to finish updating
            ch = keyboard_data->key; // Wait for user input in step mode
            if (ch == 's') {
                cpu_execute(cpu, instruction);
                cpu->csr[CSR_MINSTRET] += 1;
                sem_post(simulator->sem_refresh); // Notify display thread to refresh
            } else if (ch == 'c') {
                cpu->fast_mode = true;  // Fast mode
                cpu_execute(cpu, instruction);
                cpu->csr[CSR_MINSTRET] += 1;
                sem_post(simulator->sem_refresh);

                start_tsc = rdtsc();
                gettimeofday(&start, NULL);
            } else if (ch == 'r') {
                cpu->fast_mode = false;
                reset_system(simulator);
                sem_post(simulator->sem_refresh);
            } else if (ch == 'b') {
                cpu->fast_mode = false;
                sem_post(simulator->sem_refresh);
            } else if (ch == 'q') {
                exit(0);
            }

        } else {
            Mode mode = get_mode();
            if (mode == CPU_MODE) {
                ch = keyboard_data->key;
                if (ch == 's') {
                    cpu->fast_mode = false;
                    sem_post(simulator->sem_refresh);
                    continue;
                } else if (ch == 'r') {
                    cpu->fast_mode = false;
                    reset_system(simulator);
                    sem_post(simulator->sem_refresh);
                    continue;
                } else if (ch == 'b') {
                    cpu->fast_mode = false;
                    sem_post(simulator->sem_refresh);
                    continue;
                } else if (ch == 'q') {
                    exit(0);
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
