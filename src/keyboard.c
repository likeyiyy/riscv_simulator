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
//    pthread_mutex_lock(&mode_mutex);
    current_mode = new_mode;
//    pthread_mutex_unlock(&mode_mutex);
}

Mode get_mode(void) {
//    pthread_mutex_lock(&mode_mutex);
    Mode mode = current_mode;
//    pthread_mutex_unlock(&mode_mutex);
    return mode;
}

void process_cpu_input(KeyBoardData *data) {
    sem_post(data->sem_continue); // 通知 CPU 线程继续执行
    if (data->key == 'c' || data->key == 7) {
        // 切换到 UART 模式
        switch_mode(UART_MODE);
        sem_post(data->sem_refresh); // 通知显示线程刷新
    }
}

void process_uart_input(KeyBoardData *data) {
    if (data->key == 7) { // Ctrl+G
        // 切换回 CPU 模式
        switch_mode(CPU_MODE);
        sem_post(data->sem_refresh); // 通知显示线程刷新
    } else {
        sem_post(data->sem_refresh); // 通知显示线程刷新
        // 处理 UART 输入
        if (data->key != ERR) {
            UART *uart = data->cpu->uart;
            // 检查 FIFO 是否已满
            if (uart->fifo_count < UART_FIFO_SIZE) {
                // 将字符写入 FIFO
                mfprintf("GET KEY: %c\n", data->key);
                uart->fifo[uart->fifo_tail] = data->key;
                uart->fifo_tail = (uart->fifo_tail + 1) % UART_FIFO_SIZE;
                uart->fifo_count++;
                uart->LSR |= LSR_RX_READY; // 设置数据准备好标志
                trigger_interrupt(data->cpu, UART0_IRQ); // 触发 UART 中断
            } else {
                // FIFO 已满，丢弃输入（可以考虑其他处理方式）
            }
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
            read(STDIN_FILENO, &data->key, 1); // 读取键盘输入
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
