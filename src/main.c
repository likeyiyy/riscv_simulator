#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h> // for usleep
#include "cpu.h"
#include "memory.h"
#include "disassemble.h"

#define STACK_SIZE 32
WINDOW *create_newwin(int height, int width, int starty, int startx);

const char *reg_names2[32] = {
        "zro", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void display_registers(WINDOW *win, CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i, 1, "x%-2d (%-3s):0x%016llx", i, reg_names2[i], cpu->registers[i]);
    }
}

void display_stack(WINDOW *win, Memory *memory) {
    uint64_t base_address = 0x1000;
    for (int i = 0; i < STACK_SIZE; i++) {
        uint64_t stack_value = memory_load_dword(memory, base_address + i * 8);
        mvwprintw(win, i, 1, "0x%08lx: 0x%016llx", base_address + i * 8, stack_value);
    }
}

void display_source(WINDOW *win, Memory *memory, uint32_t pc) {
    char buffer[100];


    for (int i = 0; i < 32; i++) {
        uint32_t address = pc + i * 4;
        uint32_t instruction = memory_load_word(memory, address);
        disassemble(instruction, buffer, sizeof(buffer));
        mvwprintw(win, i, 1, "0x%08x: 0x%08x  %s", address, instruction, &buffer);
    }
}

void update_display(CPU *cpu, Memory *memory, uint32_t pc) {
//    clear();
    WINDOW *reg_win = create_newwin(32, 30, 0, 0);
    WINDOW *screen_win = create_newwin(25, 80, 0, 30);
    WINDOW *source_win = create_newwin(32, 46, 0, 110);
    WINDOW *stack_win = create_newwin(32, 33, 0, 156);

    display_registers(reg_win, cpu);
    display_stack(stack_win, memory);
    display_source(source_win, memory, pc);

    wrefresh(reg_win);
    wrefresh(screen_win);
    wrefresh(stack_win);
    wrefresh(source_win);
    refresh();
}

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

    cpu_init(&cpu);
    memory_init(&memory);

    // Initialize ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE); // Enable special keys input
    // Clear the screen
    clear();
    refresh();

    load_file_to_memory(input_file, &memory);

    // Simulate instruction execution
    int ch;
    uint32_t instruction;
    bool fast_mode = false;
    update_display(&cpu, &memory, cpu.pc);

    // 模拟指令执行
    while (cpu.pc < MEMORY_SIZE) {
        instruction = memory_load_word(&memory, cpu.pc);

        // 判断指令是否全为0
        if (instruction == 0) {
            mvprintw(32, 0, "All instructions are zero, exiting.\n");
            break;
        }

        if (!fast_mode) {
            nodelay(stdscr, FALSE); // Set blocking mode for step mode
            ch = getch(); // Wait for user input in step mode
            if (ch == 'q') break; // Quit the program
            if (ch == 's') fast_mode = false; // Step mode
            if (ch == 'f' || ch == 'c') {
                fast_mode = true;  // Fast mode
                nodelay(stdscr, TRUE); // Set back to non-blocking mode
            }

        } else {
            nodelay(stdscr, TRUE); // Set back to non-blocking mode
            usleep(1000);
        }
        cpu_execute(&cpu, &memory, instruction);

        // Update display
        update_display(&cpu, &memory, cpu.pc);


    }

    // Wait for user input before exiting
    mvprintw(33, 0, "Simulation complete. Press 'q' to exit.");
    refresh();
    if (ch != 'q') {
        // Wait for user to press 'q' to quit
        while ((ch = getch()) != 'q') {
            // Wait for user to press 'q' to quit
        }
    }


    // End ncurses mode
    endwin();
    return 0;
}


WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0); // 0, 0 gives default characters for the vertical and horizontal lines
    wrefresh(local_win);   // Show that box

    return local_win;
}
