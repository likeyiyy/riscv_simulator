#include <unistd.h> // for usleep
#include <pthread.h>
#include <sys/time.h>
#include "disassemble.h"
#include "csr.h"
#include "display.h"
#include "uart.h"
#include "mfprintf.h"

static struct timeval start;

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
    static uint64_t old_minstret;
    struct timeval end;
    long seconds, useconds;
    // 获取结束时间
    gettimeofday(&end, NULL);

    // 计算执行时间
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds * 1000.0 + useconds / 1000.0;
    start = end;
    float frequency = (cpu->csr[CSR_MINSTRET] - old_minstret) / elapsed * 1000.0 / 1024 / 1024;
    old_minstret = cpu->csr[CSR_MINSTRET];
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 1, "pc:0x%016llx", cpu->pc);
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i + 1, 1, "x%-2d (%-3s):0x%016llx", i, reg_names2[i], cpu->registers[i]);
    }
    // cpu->csr[CSR_MTVEC]
    mvwprintw(win, 33, 1, "mtvec:   0x%016llx", cpu->csr[CSR_MTVEC]);
    mvwprintw(win, 34, 1, "mstatus: 0x%016llx", cpu->csr[CSR_MSTATUS]);
    mvwprintw(win, 35, 1, "mie:   0x%016llx", cpu->csr[CSR_MIE]);
    mvwprintw(win, 36, 1, "mip:   0x%016llx", cpu->csr[CSR_MIP]);
    // display cpu.csr[CSR_MINSTRET] at the end of the win
    mvwprintw(win, 37, 1, "count:    %016llu", cpu->csr[CSR_MINSTRET]);
    mvwprintw(win, 38, 1, "frequency: %.6fMhz", frequency);
    wrefresh(win);
}

void display_stack(WINDOW *win, CPU *cpu, Memory *memory) {
    PLIC * plic = get_plic();
    static uint64_t old_sp = 0;
    if (old_sp != 0 && old_sp == cpu->registers[2]) {
        return;
    } else {
        old_sp = cpu->registers[2];
    }
    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 1, "Stack (0x%016llx):", cpu->registers[2]);
    uint64_t base_address = cpu->registers[2] - STACK_SIZE;
    for (int i = 0; i < STACK_SIZE; i++) {
        uint32_t stack_value = load_inst(memory, base_address + i * 4);
        mvwprintw(win, i + 1, 1, "0x%016llx: 0x%08lx", base_address + i * 4, stack_value);
    }
    mvwprintw(win, 33, 1, "0: 0x%08x", plic->enable[0][0]);
    mvwprintw(win, 34, 1, "0: 0x%08x", plic->enable[0][1]);
    mvwprintw(win, 35, 1, "0: 0x%08x", plic->enable[0][2]);
    mvwprintw(win, 36, 1, "0: 0x%08x", plic->enable[0][3]);

    wrefresh(win);
}

void display_source(WINDOW *win, Memory *memory, uint64_t pc) {
    static uint64_t old_pc = 0;
    if (old_pc != 0 && old_pc == pc) {
        return;
    } else {
        old_pc = pc;
    }
    wclear(win);
    box(win, 0, 0);
    char buffer[100];
    mvwprintw(win, 0, 1, "Source (0x%016llx):", pc);
    uint64_t start_pc = pc - 64;
    for (int i = 0; i < 32; i++) {
        uint64_t address = start_pc + i * 4;
        uint32_t instruction = load_inst(memory, address);
        disassemble(address, instruction, buffer, sizeof(buffer));
        if (address == pc) {
            // 启用颜色对1
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, i + 1, 1, "0x%08x>: 0x%08x  %s", address, instruction, buffer);
            wattroff(win, COLOR_PAIR(1)); // 禁用颜色对1
        } else {
            mvwprintw(win, i + 1, 1, "0x%08x : 0x%08x  %s", address, instruction, &buffer);
        }
    }
    wrefresh(win);
}

void display_screen(WINDOW *win, UART *uart) {
    static int line = 1;
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
            line = 1;
            col = 1; // Reset to the first column
            wclear(win);
            box(win, 0, 0);
        }

        uart->registers[LSR] &= ~0x01; // Clear Data Ready
        uart->registers[LSR] |= LSR_THRE; // Set Transmitter Holding Register Empty
        uart->registers[THR] = 0;
        wrefresh(win);
    }
}

void *update_display(void *arg) {
    DisplayData *data = (DisplayData *) arg;
    CPU *cpu = data->cpu;
    Memory *memory = data->memory;
    sem_t *sem_refresh = data->sem_refresh;
    sem_t *sem_continue = data->sem_continue;
    UART *uart = cpu->uart;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE); // Enable special keys input
    // Clear the screen
    clear();
    refresh();
    start_color();        // 启用颜色功能
    gettimeofday(&start, NULL);

    // 初始化颜色对 (前景色，背景色)
    init_pair(1, COLOR_RED, COLOR_BLACK);   // 颜色对1：红色文本，黑色背景

    WINDOW *screen_win = create_newwin(27, 80, 0, 30);
    WINDOW *reg_win = create_newwin(40, 30, 0, 0);
    WINDOW *source_win = create_newwin(34, 50, 0, 110);
    WINDOW *stack_win = create_newwin(40, 33, 0, 161);
    display_screen(screen_win, uart);

    display_registers(reg_win, cpu);
    display_stack(stack_win, cpu, memory);
    display_source(source_win, memory, data->pc);
    sem_post(sem_continue); // Signal main thread to proceed
    int i = 0;
    while (1) {
        if (cpu->fast_mode) {
            display_screen(screen_win, uart);

            if (i++ % 5000 == 0) {
                display_registers(reg_win, cpu);
                display_stack(stack_win, cpu, memory);
                display_source(source_win, memory, data->pc);
            }
            sem_post(sem_continue); // Signal main thread to proceed

            usleep(10); // Adjust the refresh rate as needed
            // 处理用户输入
            int ch = getch();
            if (ch != ERR) {
                uart->registers[0] = (uint8_t)ch; // 将字符写入 UART 数据寄存器
                uart->registers[LSR] |= 0x01; // 设置数据准备好标志
                trigger_interrupt(cpu, UART0_IRQ);
                mfprintf("UART0_IRQ should be happened\n");
            }
        } else {
            sem_wait(sem_refresh); // Wait for CPU thread to signal refresh
            display_screen(screen_win, uart);
            display_registers(reg_win, cpu);
            display_stack(stack_win, cpu, memory);
            display_source(source_win, memory, data->pc);
            sem_post(sem_continue); // Signal main thread to proceed
        }

    }
}

