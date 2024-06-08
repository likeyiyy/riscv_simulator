#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h> // for usleep
#include "cpu.h"
#include "memory.h"
#include "disassemble.h"
#include "display.h"
#include "uart_sim.h"


void load_file_to_memory(const char *filename, Memory *memory) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 确保内存大小足够
    if (file_size > MEMORY_SIZE) {
        fprintf(stderr, "File size exceeds memory size\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 读取文件内容到内存
    size_t bytes_read = fread(memory->data, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Failed to read complete file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    CPU cpu;
    Memory memory;
    UART uart;
    uart_init(&uart); // 初始化 UART
    memory_init(&memory);
    cpu_init(&cpu, &memory, &uart);
    init_csr_names();

    load_file_to_memory(input_file, &memory);

    sem_t sem;

    sem_init(&sem, 0, 0);

    // Initialize ncurses display thread
    DisplayData data = {&cpu, &memory, cpu.pc, &sem, &uart};
    pthread_t display_thread;
    pthread_create(&display_thread, NULL, update_display, &data);


    // Simulate instruction execution
    int ch;
    uint32_t instruction;
    bool fast_mode = false;

    while (cpu.pc < MEMORY_SIZE) {
        instruction = memory_load_word(&memory, cpu.pc);

        // 判断指令是否全为0
        if (instruction == 0) {
            mvprintw(32, 0, "All instructions are zero, exiting.\n");
            break;
        }

        if (!fast_mode) {
            sem_wait(&sem); // Wait for display thread to finish updating
            nodelay(stdscr, FALSE); // Set blocking mode for step mode
            ch = getch(); // Wait for user input in step mode
            if (ch == 'q') break; // Quit the program
            if (ch == 's') fast_mode = false; // Step mode
            if (ch == 'f' || ch == 'c') {
                fast_mode = true;  // Fast mode
                nodelay(stdscr, TRUE); // Set back to non-blocking mode
            }
        } else {

        }
        cpu_execute(&cpu, &memory, instruction);

        // Update PC for display
        data.pc = cpu.pc;
    }

    // Wait for user input before exiting
    mvprintw(33, 0, "Simulation complete. Press 'q' to exit.");
    refresh();
    while ((ch = getch()) != 'q') {
        // Wait for user to press 'q' to quit
    }

    // End ncurses mode
    endwin();
    // Cancel the display thread
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    sem_destroy(&sem);

    return 0;
}
