#include "keyboard.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>



Mode current_mode = CPU_MODE;
pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;

void switch_mode(Mode new_mode) {
    pthread_mutex_lock(&mode_mutex);
    current_mode = new_mode;
    pthread_mutex_unlock(&mode_mutex);
}

Mode get_mode(void) {
    pthread_mutex_lock(&mode_mutex);
    Mode mode = current_mode;
    pthread_mutex_unlock(&mode_mutex);
    return mode;
}

void process_cpu_input(KeyBoardData *data) {
    printf("key: %d\n", data->key);
    sem_post(data->sem_continue); // 通知 CPU 线程继续执行
    if (data->key == 'c') {
        // 切换到 UART 模式
        switch_mode(UART_MODE);
    }
}

void process_uart_input(KeyBoardData *data) {
    if (data->key == 7) { // Ctrl+G
        // 切换回 CPU 模式
        switch_mode(CPU_MODE);
    } else {
        // 处理 UART 输入
        int ch = data->key;
        if (ch != ERR) {
            data->cpu->uart->registers[0] = (uint8_t)ch; // 将字符写入 UART 数据寄存器
            data->cpu->uart->registers[LSR] |= 0x01; // 设置数据准备好标志
            trigger_interrupt(data->cpu, UART0_IRQ); // 触发 UART 中断
        }
    }
}

void* keyboard_input(void *arg) {
    KeyBoardData *data = (KeyBoardData *) arg;
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // 获取终端当前属性
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // 设置终端为非规范模式，关闭回显
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // 应用新属性

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    while (1) {
        int ret = poll(fds, 1, -1);
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            char ch;
            read(STDIN_FILENO, &ch, 1); // 读取键盘输入
            data->key = ch;
            Mode mode = get_mode();

            if (mode == CPU_MODE) {
                process_cpu_input(data);
            } else if (mode == UART_MODE) {
                process_uart_input(data);
            }
        }
    }

    // 恢复终端属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return NULL;
}
