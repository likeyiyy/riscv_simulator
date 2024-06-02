#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h> // for usleep
#include "cpu.h"
#include "memory.h"
#include "disassemble.h"

#define STACK_SIZE 32
WINDOW *create_newwin(int height, int width, int starty, int startx);

void display_registers(WINDOW *win, CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i, 1, "x%-2d: 0x%016llx", i, cpu->registers[i]);
    }
}

void display_stack(WINDOW *win, Memory *memory) {
    for (int i = 0; i < STACK_SIZE; i++) {
        uint64_t stack_value = memory_load_word(memory, MEMORY_SIZE - (i + 1) * 8);
        mvwprintw(win, i, 1, "stack[%2d]: 0x%016llx", i, stack_value);
    }
}

void display_source(WINDOW *win, Memory *memory, uint32_t pc) {
    char buffer[100];


    for (int i = 0; i < 32; i++) {
        uint32_t address = pc + i * 4;
        uint32_t instruction = memory_load_word(memory, address);
        disassemble(instruction, buffer, sizeof(buffer));
        mvwprintw(win, i, 1, "0x%08x: 0x%08x    %s", address, instruction, &buffer);
    }
}

void update_display(CPU *cpu, Memory *memory, uint32_t pc) {
//    clear();
    WINDOW *reg_win = create_newwin(32, 26, 0, 0);
    WINDOW *screen_win = create_newwin(25, 80, 0, 28);
    WINDOW *stack_win = create_newwin(32, 31, 0, 160);
    WINDOW *source_win = create_newwin(32, 50, 0, 110);

    display_registers(reg_win, cpu);
    display_stack(stack_win, memory);
    display_source(source_win, memory, pc);

    wrefresh(reg_win);
    wrefresh(screen_win);
    wrefresh(stack_win);
    wrefresh(source_win);
    refresh();
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

    // 打开包含指令的文件
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // 从文件中读取指令并加载到内存
    uint32_t address = 0;
    uint32_t instruction;
    while (fread(&instruction, sizeof(uint32_t), 1, file)) {
        memory_store_word(&memory, address, instruction);
        address += 4;
    }

    fclose(file);
    // Simulate instruction execution
    int ch;
    bool fast_mode = false;
    update_display(&cpu, &memory, cpu.pc);

    // 模拟指令执行
    while (cpu.pc < MEMORY_SIZE) {
        ch = getch();
        if (ch == 'q') break; // Quit the program
        if (ch == 's') fast_mode = false; // Step mode
        if (ch == 'f') fast_mode = true;  // Fast mode

        instruction = memory_load_word(&memory, cpu.pc);

        // 判断指令是否全为0
        if (instruction == 0) {
            mvprintw(32, 0, "All instructions are zero, exiting.\n");
            break;
        }
        cpu_execute(&cpu, instruction);
        cpu.pc += 4; // 下一条指令

        // Update display
        update_display(&cpu, &memory, cpu.pc);

        if (!fast_mode) {
            nodelay(stdscr, FALSE); // Set blocking mode for step mode
            getch(); // Wait for user input in step mode
            nodelay(stdscr, TRUE); // Set back to non-blocking mode
        } else {
            nodelay(stdscr, TRUE); // Set back to non-blocking mode
            usleep(1000);
        }
    }

    // Wait for user input before exiting
    mvprintw(33, 0, "Simulation complete. Press 'q' to exit.");
    refresh();
    while ((ch = getch()) != 'q') {
        // Wait for user to press 'q' to quit
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
