#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h> // for usleep
#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>

#include "cpu.h"
#include "memory.h"
#include "disassemble.h"
#include "display.h"
#include "uart.h"
#include "helper.h"
#include "mfprintf.h"
#include "csr.h"
#include "exception.h"
#include "keyboard.h"
#include "simulator.h"


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
    if (file_size + address > MEMORY_SIZE) {
        fprintf(stderr, "File size exceeds memory size or invalid address\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 读取文件内容到内存
    size_t bytes_read = fread(memory->data + address, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Failed to read complete file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}


int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    uint64_t load_address = 0;
    uint64_t end_address = MEMORY_SIZE;

    if (parse_arguments(argc, argv, &input_file, &load_address, &end_address) != 0) {
        print_usage(argv[0]);
        return 1;
    } else {
        printf("Input file: %s\n", input_file);
        printf("Load address: 0x%lx\n", load_address);
        printf("End address: 0x%lx\n", end_address);
    }

    CPU cpu;
    Memory memory;

    CLINT *clint = get_clint();
    clint_init(clint);

    PLIC *plic = get_plic();
    plic_init(plic);

    UART* uart = get_uart();
    uart_init(uart); // 初始化 UART


    memory_init(&memory);
    cpu_init(&cpu, &memory, clint, plic, uart);
    init_csr_names();

    load_file_to_memory(input_file, &memory, load_address);
    cpu.pc = load_address;

    sem_t sem_refresh;
    sem_t sem_continue;

    sem_init(&sem_refresh, 0, 0);
    sem_init(&sem_continue, 0, 0);

    pthread_t display_thread;
    pthread_t keyboard_thread;
    pthread_t simulator_thread;

    // Initialize ncurses display thread
    DisplayData data = {&cpu, &memory, &sem_refresh};
    KeyBoardData keyboard_data = {&cpu, -1, &sem_continue};
    Simulator simulator = {&cpu, &memory, &data, &keyboard_data, &sem_continue, &sem_refresh, end_address};

    pthread_create(&display_thread, NULL, update_display, &data);
    pthread_create(&keyboard_thread, NULL, keyboard_input, &keyboard_data);
    pthread_create(&simulator_thread, NULL, cpu_simulator, &simulator);

    pthread_join(display_thread, NULL);
    pthread_join(keyboard_thread, NULL);
    pthread_join(simulator_thread, NULL);

    // End ncurses mode
    endwin();
    sem_destroy(&sem_continue);
    sem_destroy(&sem_refresh);

    return 0;
}
