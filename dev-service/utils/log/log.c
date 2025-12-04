#include "log.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static const char* level_str[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

// ANSI 颜色码
static const char* level_color[] = {
    "\033[34m", // DEBUG 蓝
    "\033[32m", // INFO 绿
    "\033[33m", // WARN 黄
    "\033[31m"  // ERROR 红
};

static FILE *log_fp = NULL;

int log_init(const char *path)
{
    if (!path) 
        return -1;

    log_fp = fopen(path, "a");
    if (!log_fp) 
    {
        printf("log_set_file: 打开文件失败: %s\n", path);
        return -1;
    }
    return 0;
}

void log_close(void)
{
    if (log_fp) 
    {
        fclose(log_fp);
        log_fp = NULL;
    }
}

void log_print(log_level_t level, const char *file, int line, const char *fmt, ...)
{
    if(level < LOG_DEBUG || level > LOG_ERROR) 
        return;

    // 时间戳
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, fmt);

    // 先打印到终端（带颜色）
    printf("%s[%s] [%s] [%s:%d] ", level_color[level], time_buf, level_str[level], file, line);
    vprintf(fmt, args);
    printf("\033[0m\n"); // 重置颜色

    // 打印到文件（不带颜色）
    if(log_fp) 
    {
        fprintf(log_fp, "[%s] [%s] [%s:%d] ", time_buf, level_str[level], file, line);
        va_list args_copy;
        va_copy(args_copy, args);
        vfprintf(log_fp, fmt, args_copy);
        va_end(args_copy);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }

    va_end(args);
}
