#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

void log_print(log_level_t level, const char *file, int line, const char *func, const char *fmt, ...);
int log_init(const char *path);
void log_close(void);

#define LOGD(fmt, ...) log_print(LOG_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) log_print(LOG_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) log_print(LOG_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) log_print(LOG_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#endif // LOG_H
