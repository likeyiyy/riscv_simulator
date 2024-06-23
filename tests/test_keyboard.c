#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <pthread.h>
#include <ncurses.h>

// 定义模式
typedef enum {
    CPU_MODE,
    UART_MODE
} Mode;

Mode current_mode = CPU_MODE;
pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;

void switch_mode(Mode new_mode) {
    pthread_mutex_lock(&mode_mutex);
    current_mode = new_mode;
    pthread_mutex_unlock(&mode_mutex);
}

Mode get_mode() {
    pthread_mutex_lock(&mode_mutex);
    Mode mode = current_mode;
    pthread_mutex_unlock(&mode_mutex);
    return mode;
}

void process_cpu_input(int key) {
    if (key == 's') {
        // 单步调试
        mvprintw(0, 0, "Single step");
    } else if (key == 'c') {
        // 切换到 UART 模式
        mvprintw(1, 0, "Switching to UART mode");
        switch_mode(UART_MODE);
    }
    refresh();
}

void process_uart_input(int key) {
    if (key == 7) { // Ctrl+G
        // 切换回 CPU 模式
        mvprintw(2, 0, "Switching to CPU mode");
        switch_mode(CPU_MODE);
    } else {
        // 处理 UART 输入
        mvprintw(3, 0, "UART received: %c", key);
    }
    refresh();
}

void* keyboard_input(void *arg) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // 获取终端当前属性
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // 设置终端为非规范模式，关闭回显
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // 应用新属性

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    initscr();
    cbreak();
    noecho();  // 禁用字符回显
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    while (1) {
        int ret = poll(fds, 1, -1);
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            Mode mode = get_mode();
            if (mode == CPU_MODE) {
                process_cpu_input(ch);
            } else if (mode == UART_MODE) {
                process_uart_input(ch);
            }
        }
    }

    // 恢复终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    endwin();
    return NULL;
}

void* cpu_simulator(void *arg) {
    while (1) {
        // 模拟 CPU 的其他任务
        usleep(500000);
    }
    return NULL;
}

int main() {
    pthread_t input_thread, cpu_thread;

    // 创建键盘输入线程
    pthread_create(&input_thread, NULL, keyboard_input, NULL);

    // 创建 CPU 模拟器线程
    pthread_create(&cpu_thread, NULL, cpu_simulator, NULL);

    // 等待线程完成
    pthread_join(input_thread, NULL);
    pthread_join(cpu_thread, NULL);

    return 0;
}
