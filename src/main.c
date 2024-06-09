#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h> // for usleep
#include <fcntl.h>
#include <getopt.h>

#include "cpu.h"
#include "memory.h"
#include "disassemble.h"
#include "display.h"
#include "uart_sim.h"
#include "helper.h"
#include "mfprintf.h"


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
    // print argc and argv
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

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
    UART uart;
    uart_init(&uart); // 初始化 UART
    memory_init(&memory);
    cpu_init(&cpu, &memory, &uart);
    init_csr_names();

    load_file_to_memory(input_file, &memory, load_address);
    cpu.pc = load_address;

    sem_t sem_refresh;
    sem_t sem_continue;

    sem_init(&sem_refresh, 0, 0);
    sem_init(&sem_continue, 0, 0);

    // Initialize ncurses display thread
    DisplayData data = {&cpu, &memory, cpu.pc, &sem_refresh, &sem_continue, &uart};
    pthread_t display_thread;
    pthread_create(&display_thread, NULL, update_display, &data);


    // Simulate instruction execution
    int ch;
    uint32_t instruction;

    while (cpu.pc < MEMORY_SIZE) {
        instruction = memory_load_word(&memory, cpu.pc);
        mfprintf("Address: 0x%016lx, Instruction: %08x\n", cpu.pc, instruction);


        // 判断指令是否全为0
        if (instruction == 0) {
            mvprintw(32, 0, "All instructions are zero, exiting.\n");
            break;
        }

        if (!cpu.fast_mode) {
            sem_wait(&sem_continue); // Wait for display thread to finish updating
            nodelay(stdscr, FALSE); // Set blocking mode for step mode
            ch = getch(); // Wait for user input in step mode
            if (ch == 'q') break; // Quit the program
            if (ch == 's') {
                cpu.fast_mode = false; // Step mode
                sem_post(&sem_refresh); // Notify display thread to refresh
            }
            if (ch == 'f' || ch == 'c') {
                cpu.fast_mode = true;  // Fast mode
                sem_post(&sem_refresh);
                nodelay(stdscr, TRUE); // Set back to non-blocking mode
            }
            cpu_execute(&cpu, &memory, instruction);

        } else {
            if (cpu.pc == end_address) {
                cpu.fast_mode = false;
                sem_post(&sem_refresh);
            } else {
                cpu_execute(&cpu, &memory, instruction);
            }

        }
        data.pc = cpu.pc;
    }


    // Wait for user input before exiting
    mvprintw(34, 0, "Simulation complete. Press 'q' to exit.");
    refresh();
    while ((ch = getch()) != 'q') {
        // Wait for user to press 'q' to quit
    }

    // End ncurses mode
    endwin();
    // Cancel the display thread
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    sem_destroy(&sem_continue);
    sem_destroy(&sem_refresh);

    return 0;
}
