#include "mfprintf.h"

// 实现 mfprintf 函数
int mfprintf(const char *format, ...) {
    // 打开文件以追加模式写入
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL) {
        return -1; // 如果文件无法打开，返回错误码 -1
    }

    // 定义 va_list 变量来处理可变参数
    va_list args;
    va_start(args, format);

    // 使用 vfprintf 将格式化字符串写入文件
    int result = vfprintf(file, format, args);

    // 清理 va_list
    va_end(args);
    // 关闭文件
    fclose(file);

    return result; // 返回写入的字符数
}
