#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// 定义缓冲区大小
#define MAX_LINES 18
#define LINE_LENGTH 256

// 循环缓冲区，用于存储最新的行
char buffer[MAX_LINES][LINE_LENGTH];
int buffer_start = 0;
int buffer_count = 0;

void* ncurses_thread(void* arg) {
    FILE *file = fopen("output.log", "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file for reading.\n");
        return NULL;
    }

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    WINDOW *win = newwin(20, 80, 0, 0); // 创建新窗口
    box(win, 0, 0);                     // 绘制边框
    wrefresh(win);

    long last_pos = 0; // 上次读取的位置

    while (1) {
        // 移动到上次读取的位置
        fseek(file, last_pos, SEEK_SET);

        char line[LINE_LENGTH];
        while (fgets(line, sizeof(line), file)) {
            // 将新行添加到缓冲区
            strncpy(buffer[(buffer_start + buffer_count) % MAX_LINES], line, LINE_LENGTH - 1);
            buffer[(buffer_start + buffer_count) % MAX_LINES][LINE_LENGTH - 1] = '\0'; // 确保字符串以 null 结尾
            if (buffer_count < MAX_LINES) {
                buffer_count++;
            } else {
                buffer_start = (buffer_start + 1) % MAX_LINES;
            }
        }

        // 记录当前文件指针位置
        last_pos = ftell(file);

        // 清空窗口内容
        werase(win);
        box(win, 0, 0);

        // 打印缓冲区中的行
        for (int i = 0; i < buffer_count; i++) {
            mvwprintw(win, i + 1, 1, "%s", buffer[(buffer_start + i) % MAX_LINES]);
        }

        wrefresh(win);
        usleep(500000); // 0.5秒间隔更新
    }

    endwin();
    fclose(file);
    return NULL;
}

int main() {
    pthread_t thread;

    // 打开文件
    FILE *file = fopen("output.log", "a+");
    if (!file) {
        perror("fopen");
        return 1;
    }

    fprintf(file, "This will be written to output.log\n");
    fflush(file); // 确保输出立即写入文件

    // 启动 ncurses 线程
    pthread_create(&thread, NULL, ncurses_thread, NULL);

    // 主线程继续运行其他任务
    for (int i = 0; i < 100; ++i) {
        fprintf(file, "Log entry %d\n", i);
        fflush(file); // 确保输出立即写入文件
        sleep(1); // 模拟日志生成间隔
    }

    pthread_join(thread, NULL);
    fclose(file); // 关闭文件

    return 0;
}
