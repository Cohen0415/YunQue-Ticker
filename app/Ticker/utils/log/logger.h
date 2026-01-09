#ifndef LOGGER_H
#define LOGGER_H

#include <QtGlobal>
#include <QString>
#include <QDebug>
#include <cstdarg>
#include <cstdio>

// 日志等级
enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

// 全局日志等级（默认 DEBUG）
extern LogLevel g_logLevel;

// 安装自定义 Qt 日志处理器
void installCustomLogger();

// printf 风格日志函数
void customLog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...);

// 方便宏
#define LOG_DEBUG(fmt, ...) \
    do { if (LogLevel::DEBUG >= g_logLevel) customLog(LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_INFO(fmt, ...) \
    do { if (LogLevel::INFO >= g_logLevel) customLog(LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_WARN(fmt, ...) \
    do { if (LogLevel::WARN >= g_logLevel) customLog(LogLevel::WARN, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_ERROR(fmt, ...) \
    do { if (LogLevel::ERROR >= g_logLevel) customLog(LogLevel::ERROR, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); } while(0)

#endif // LOGGER_H
