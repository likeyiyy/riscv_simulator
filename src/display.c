#include <unistd.h> // for usleep
#include "disassemble.h"
#include "display.h"

const char *reg_names2[32] = {
        "zro", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0); // 0, 0 gives default characters for the vertical and horizontal lines
    wrefresh(local_win);   // Show that box

    return local_win;
}

void display_registers(WINDOW *win, CPU *cpu) {
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i, 1, "x%-2d (%-3s):0x%016llx", i, reg_names2[i], cpu->registers[i]);
    }
}

void display_stack(WINDOW *win, CPU *cpu, Memory *memory) {
    uint64_t base_address = cpu->registers[2] - STACK_SIZE * 4;
    for (int i = 0; i < STACK_SIZE; i++) {
        uint64_t stack_value = memory_load_dword(memory, base_address + i * 8);
        mvwprintw(win, i, 1, "0x%08lx: 0x%016llx", base_address + i * 8, stack_value);
    }
}

void display_source(WINDOW *win, Memory *memory, uint64_t pc) {
    char buffer[100];

    for (int i = 0; i < 32; i++) {
        uint64_t address = pc + i * 4;
        uint32_t instruction = memory_load_word(memory, address);
        disassemble(address, instruction, buffer, sizeof(buffer));
        mvwprintw(win, i, 1, "0x%08x: 0x%08x  %s", address, instruction, &buffer);
    }
}

void *update_display(void *arg) {
    DisplayData *data = (DisplayData *)arg;
    CPU *cpu = data->cpu;
    Memory *memory = data->memory;
    sem_t *sem = data->sem;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE); // Enable special keys input
    // Clear the screen
    clear();
    refresh();

    while (1) {
        WINDOW *reg_win = create_newwin(32, 30, 0, 0);
        WINDOW *screen_win = create_newwin(25, 80, 0, 30);
        WINDOW *source_win = create_newwin(32, 46, 0, 110);
        WINDOW *stack_win = create_newwin(32, 33, 0, 156);

        display_registers(reg_win, cpu);
        display_stack(stack_win, cpu, memory);
        display_source(source_win, memory, data->pc);

        wrefresh(reg_win);
        wrefresh(screen_win);
        wrefresh(stack_win);
        wrefresh(source_win);
        refresh();
        sem_post(sem); // Signal main thread to proceed
        usleep(50000); // Adjust the refresh rate as needed
    }

    endwin();
    return NULL;
}
