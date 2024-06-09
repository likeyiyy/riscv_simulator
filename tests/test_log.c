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

// 自定义函数，将格式化字符串写入指定文件
void fprint_to_file(const char *filename, const char *format, ...) {
    FILE *file = fopen(filename, "a"); // 以追加模式打开
    if (file == NULL) {
        perror("fopen");
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fclose(file);
}

void* ncurses_thread(void* arg) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    WINDOW *win = newwin(20, 80, 0, 0); // 创建新窗口
    box(win, 0, 0);                     // 绘制边框
    wrefresh(win);

    FILE *file;
    long last_pos = 0; // 上次读取的位置

    while (1) {
        file = fopen("output.log", "r");
        if (!file) {
            mvwprintw(win, 1, 1, "Error: Unable to open file.");
            wrefresh(win);
            getch();
            endwin();
            return NULL;
        }

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
        fclose(file);
        usleep(500000); // 0.5秒间隔更新
    }

    endwin();
    return NULL;
}

int main() {
    pthread_t thread;

    fprint_to_file("output.log", "This will be written to output.log\n");

    // 启动 ncurses 线程
    pthread_create(&thread, NULL, ncurses_thread, NULL);

    // 主线程继续运行其他任务
    for (int i = 0; i < 100; ++i) {
        fprint_to_file("output.log", "Log entry %d\n", i);
        fflush(stdout); // 确保输出立即写入文件
        sleep(0.1); // 模拟日志生成间隔
    }

    pthread_join(thread, NULL);
    return 0;
}
