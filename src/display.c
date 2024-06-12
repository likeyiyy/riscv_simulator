#include <unistd.h> // for usleep
#include <pthread.h>
#include <sys/time.h>
#include "disassemble.h"
#include "csr.h"
#include "display.h"
#include "uart.h"
#include "mfprintf.h"
#include "keyboard.h"

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
    mvwprintw(win, 0, 1, "PC:0x%016lx", cpu->pc);
    for (int i = 0; i < 32; i++) {
        mvwprintw(win, i + 1, 1, "x%-2d (%-3s):0x%016lx", i, reg_names2[i], cpu->registers[i]);
    }
    uint8_t csr_base_y_index = 33;

    const char *CPU_MODES[] = {
            "User", "Supervisor", "Reserved", "Machine"
    };
    mvwprintw(win, csr_base_y_index++, 1, "cpu mode: %s", CPU_MODES[cpu->priv]);
    mvwprintw(win, csr_base_y_index++, 1, "mhartid:  0x%016lx", cpu->csr[CSR_MHARTID]);
    mvwprintw(win, csr_base_y_index++, 1, "mtvec:    0x%016lx", cpu->csr[CSR_MTVEC]);
    mvwprintw(win, csr_base_y_index++, 1, "mstatus:  0x%016lx", cpu->csr[CSR_MSTATUS]);
    mvwprintw(win, csr_base_y_index++, 1, "mie:      0x%016lx", cpu->csr[CSR_MIE]);
    mvwprintw(win, csr_base_y_index++, 1, "mcause:   0x%016lx", cpu->csr[CSR_MCAUSE]);
    mvwprintw(win, csr_base_y_index++, 1, "mip:      0x%016lx", cpu->csr[CSR_MIP]);
    mvwprintw(win, csr_base_y_index++, 1, "minstret: 0x%016lu", cpu->csr[CSR_MINSTRET]);
    mvwprintw(win, csr_base_y_index++, 1, "freq:     %.2fMhz", frequency);

    wrefresh(win);
}

void display_stack(WINDOW *win, CPU *cpu, Memory *memory) {
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

void display_keyboard_mode(WINDOW *win) {
    static Mode old_mode = NONE_MODE;
    Mode mode = get_mode();
    if (old_mode == mode) {
        return;
    } else {
        old_mode = mode;
    }
    wclear(win);
    box(win, 0, 0);
    if (mode == CPU_MODE) {
        mvwprintw(win, 0, 1, "KeyBoard Mode: CPU Mode");
        mvwprintw(win, 1, 1, "s: step, c: continue, b: break, r: reset, q: quit");
    } else {
        mvwprintw(win, 0, 1, "KeyBoard Mode: UART Mode");
        mvwprintw(win, 1, 1, "Ctrl+G: switch to CPU mode");
    }
    wrefresh(win);
}

void display_screen(DisplayData* display_data, WINDOW *win, UART *uart) {
    mvwprintw(win, 0, 1, "Screen(80*25)");
    if ((uart->LSR & LSR_TX_IDLE) == 0) { // Check if is idle, if not idle, mean that data is ok
        uint8_t value = uart->THR;
        char buffer[2] = {value, '\0'};

        if (value == '\n') { // Handle newline character
            display_data->line++;
            display_data->col = 1; // Reset to the first column
        } else {
            mvwprintw(win, display_data->line, display_data->col++, "%s", buffer);
            if (display_data->col >= getmaxx(win) - 1) { // Move to the next line if end of line is reached
                display_data->line++;
                display_data->col = 1; // Reset to the first column
            }
        }

        if (display_data->line >= getmaxy(win) - 1) { // Clear the window if the end is reached
            display_data->line = 1;
            display_data->col = 1; // Reset to the first column
            wclear(win);
            box(win, 0, 0);
        }

        uart->LSR |= LSR_THRE; // Set Transmitter Holding Register Empty
        uart->THR = 0;
    }
    wrefresh(win);
}

void display_uart(WINDOW *win, UART *uart) {
    mvwprintw(win, 0, 1, "UART Registers");

    // 寄存器名称数组
    const char *register_names[] = {
            "THR", "RBR", "IER", "IIR",
            "FCR", "LCR", "MCR", "LSR",
            "MSR", "SCR", "DLL", "DLM"
    };

    // 寄存器值数组
    uint32_t register_values[] = {
            uart->THR, uart->RBR, uart->IER, uart->IIR,
            uart->FCR, uart->LCR, uart->MCR, uart->LSR,
            uart->MSR, uart->SCR, uart->DLL, uart->DLM
    };

    // 显示 UART 寄存器的值，分成两列
    for (int i = 0; i < 6; ++i) {
        mvwprintw(win, i + 1, 1, "%-4s: 0x%02x", register_names[i], register_values[i]);
        mvwprintw(win, i + 1, 18, "%-4s: 0x%02x", register_names[i + 6], register_values[i + 6]);
    }

    wrefresh(win);
}

void display_plic(WINDOW *win, PLIC *plic) {

    mvwprintw(win, 0, 1, "PLIC Registers (hart0)");

    // 显示 priority id=10 的优先级
    mvwprintw(win, 1, 1, "Priority [id=10]: %u", plic->priority[10]);

    // 显示 pending 寄存器的值
    for (int i = 0; i < MAX_INTERRUPTS / 32; ++i) {
        mvwprintw(win, 2 + i, 1, "Pending [%d]: 0x%08x", i, plic->pending[i]);
    }

    // 显示 hart0 的 threshold 寄存器的值
    mvwprintw(win, 6, 1, "Threshold [hart0]: %u", plic->threshold[0]);

    // 显示 hart0 的 enable 位图
    for (int i = 0; i < MAX_INTERRUPTS / 32; ++i) {
        mvwprintw(win, 7 + i, 1, "Enable [hart0][%d]: 0x%08x", i, plic->enable[0][i]);
    }

    // 显示 hart0 的 claim/complete 寄存器的值
    mvwprintw(win, 11, 1, "Claim/Complete [hart0]: 0x%08x", plic->claim_complete[0]);

    wrefresh(win);
}


void *update_display(void *arg) {
    DisplayData *display = (DisplayData *) arg;
    CPU *cpu = display->cpu;
    Memory *memory = display->memory;
    sem_t *sem_refresh = display->sem_refresh;
    UART *uart = cpu->uart;

    initscr();
    start_color();        // 启用颜色功能
    gettimeofday(&start, NULL);

    // 初始化颜色对 (前景色，背景色)
    init_pair(1, COLOR_RED, COLOR_BLACK);   // 颜色对1：红色文本，黑色背景

    WINDOW *reg_win = create_newwin(REG_WIN_HEIGHT, REG_WIN_WIDTH, 0, REG_WIN_START_X);
    WINDOW *status_win = create_newwin(STATUS_WIN_HEIGHT, SCREEN_WIN_WIDTH, 0, SCREEN_WIN_START_X);
    WINDOW *screen_win = create_newwin(SCREEN_WIN_HEIGHT, SCREEN_WIN_WIDTH, STATUS_WIN_HEIGHT, SCREEN_WIN_START_X);
    WINDOW *uart_win = create_newwin(UART_WIN_HEIGHT, UART_WIN_WIDTH, STATUS_WIN_HEIGHT + SCREEN_WIN_HEIGHT,
                                     SCREEN_WIN_START_X);
    WINDOW *plic_win = create_newwin(PLIC_WIN_HEIGHT, PLIC_WIN_WIDTH, STATUS_WIN_HEIGHT + SCREEN_WIN_HEIGHT,
                                     SCREEN_WIN_START_X + UART_WIN_WIDTH);
    WINDOW *source_win = create_newwin(SOURCE_WIN_HEIGHT, SOURCE_WIN_WIDTH, 0, SOURCE_WIN_START_X);
    WINDOW *stack_win = create_newwin(STACK_WIN_HEIGHT, STACK_WIN_WIDTH, 0, STACK_WIN_START_X);
    display->screen_win = screen_win;
    display->line = 1;
    display->col = 1;

    display_registers(reg_win, cpu);
    display_screen(display, screen_win, uart);
    display_uart(uart_win, uart);
    display_plic(plic_win, cpu->plic);
    display_keyboard_mode(status_win);
    display_stack(stack_win, cpu, memory);
    display_source(source_win, memory, display->cpu->pc);

    int i = 0;
    while (1) {
        if (cpu->fast_mode) {
            display_screen(display, screen_win, uart);
            if (i % 500 == 0) {
                display_uart(uart_win, uart);
                display_plic(plic_win, cpu->plic);
            }

            if (i++ % 5000 == 0) {
                display_registers(reg_win, cpu);
                display_keyboard_mode(status_win);
                display_stack(stack_win, cpu, memory);
                display_source(source_win, memory, display->cpu->pc);
            }
            usleep(10); // Adjust the refresh rate as needed

        } else {
            sem_wait(sem_refresh);
            display_registers(reg_win, cpu);
            display_keyboard_mode(status_win);
            display_screen(display, screen_win, uart);
            display_uart(uart_win, uart);
            display_plic(plic_win, cpu->plic);
            display_stack(stack_win, cpu, memory);
            display_source(source_win, memory, display->cpu->pc);
            usleep(100000); // Adjust the refresh rate as needed
        }
    }
}

