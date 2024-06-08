#include <unistd.h> // for usleep
#include "disassemble.h"
#include "display.h"
#include "uart_sim.h"


const char *reg_names2[32] = {
        "zro", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); // 0, 0 gives default characters for the vertical and horizontal lines
    wrefresh(local_win);   // Show that box

    return local_win;
}

void display_registers(WINDOW *win, CPU *cpu) {
    mvwprintw(win, 0, 1, "pc:0x%016llx", cpu->pc);
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i + 1, 1, "x%-2d (%-3s):0x%016llx", i, reg_names2[i], cpu->registers[i]);
    }
}

void display_stack(WINDOW *win, CPU *cpu, Memory *memory) {
    mvwprintw(win, 0, 1, "Stack (0x%016llx):", cpu->registers[2]);
    uint64_t base_address = cpu->registers[2] - STACK_SIZE;
    for (int i = 0; i < STACK_SIZE; i++) {
        uint32_t stack_value = memory_load_word(memory, base_address + i * 4);
        mvwprintw(win, i + 1, 1, "0x%016llx: 0x%08lx", base_address + i * 4, stack_value);
    }
}

void display_source(WINDOW *win, Memory *memory, uint64_t pc) {
    char buffer[100];
    mvwprintw(win, 0, 1, "Source (0x%016llx):", pc);
    for (int i = 0; i < 32; i++) {
        uint64_t address = pc + i * 4;
        uint32_t instruction = memory_load_word(memory, address);
        disassemble(address, instruction, buffer, sizeof(buffer));
        mvwprintw(win, i + 1, 1, "0x%08x: 0x%08x  %s", address, instruction, &buffer);
    }
}

void display_screen(WINDOW *win, UART *uart) {
    static int line = 0;
    static int col = 1; // Start from column 1 to leave space for the box
    if (uart->registers[LSR] & 0x01) { // Check if data is ready
        uint8_t value = uart->registers[RHR];
        char buffer[2] = {value, '\0'};

        if (value == '\n') { // Handle newline character
            line++;
            col = 1; // Reset to the first column
        } else {
            mvwprintw(win, line, col++, "%s", buffer);
            if (col >= getmaxx(win) - 1) { // Move to the next line if end of line is reached
                line++;
                col = 1; // Reset to the first column
            }
        }

        if (line >= getmaxy(win) - 1) { // Clear the window if the end is reached
            line = 0;
            col = 1; // Reset to the first column
            wclear(win);
            box(win, 0, 0);
        }

        uart->registers[LSR] &= ~0x01; // Clear Data Ready
        uart->registers[LSR] |= LSR_THRE; // Set Transmitter Holding Register Empty
        uart->registers[THR] = 0;
    }
    wrefresh(win); // Refresh the window to display changes
}

void *update_display(void *arg) {
    DisplayData *data = (DisplayData *) arg;
    CPU *cpu = data->cpu;
    Memory *memory = data->memory;
    sem_t *sem = data->sem;
    UART *uart = data->uart;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE); // Enable special keys input
    // Clear the screen
    clear();
    refresh();

    WINDOW *reg_win = create_newwin(33, 30, 0, 0);
    WINDOW *screen_win = create_newwin(26, 80, 0, 30);
    WINDOW *source_win = create_newwin(33, 46, 0, 110);
    WINDOW *stack_win = create_newwin(33, 33, 0, 156);


    while (1) {

        display_registers(reg_win, cpu);
        display_stack(stack_win, cpu, memory);
        display_source(source_win, memory, data->pc);
        display_screen(screen_win, uart);

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
