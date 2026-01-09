#include "logger.h"
#include <QDateTime>
#include <QMessageLogContext>
#include <QString>
#include <cstdarg>
#include <cstdio>

// ANSI 颜色码
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// 全局日志等级定义
LogLevel g_logLevel = LogLevel::DEBUG;

static const char* logLevelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO:  return "INFO ";
    case LogLevel::WARN:  return "WARN ";
    case LogLevel::ERROR: return "ERROR";
    default:              return "UNKN ";
    }
}

// 安装 Qt 自定义消息处理器
void installCustomLogger()
{
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg){
        const char* color = ANSI_COLOR_CYAN;
        LogLevel level = LogLevel::DEBUG;
        switch(type) {
            case QtDebugMsg:    level = LogLevel::DEBUG; color = ANSI_COLOR_CYAN; break;
            case QtInfoMsg:     level = LogLevel::INFO;  color = ANSI_COLOR_GREEN; break;
            case QtWarningMsg:  level = LogLevel::WARN;  color = ANSI_COLOR_YELLOW; break;
            case QtCriticalMsg: level = LogLevel::ERROR; color = ANSI_COLOR_RED; break;
            case QtFatalMsg:    level = LogLevel::ERROR; color = ANSI_COLOR_RED; break;
        }
        if(level >= g_logLevel){
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            QString func = context.function ? context.function : "unknown_func";
            fprintf(stderr, "%s[%s] [%d:%s] %s%s\n",
                    color, timestamp.toLocal8Bit().data(),
                    context.line, func, msg.toLocal8Bit().data(),
                    ANSI_COLOR_RESET);
        }
    });
}

// printf 风格自定义日志
void customLog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...)
{
    if(level < g_logLevel) return;

    va_list args;
    va_start(args, fmt);

    const char* color = ANSI_COLOR_CYAN;
    switch(level){
        case LogLevel::DEBUG: color = ANSI_COLOR_CYAN; break;
        case LogLevel::INFO:  color = ANSI_COLOR_GREEN; break;
        case LogLevel::WARN:  color = ANSI_COLOR_YELLOW; break;
        case LogLevel::ERROR: color = ANSI_COLOR_RED; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    fprintf(stderr, "%s[%s] [%s] [%s:%d:%s] ",
            color,
            timestamp.toLocal8Bit().data(),
            logLevelToString(level),
            file,
            line,
            func);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "%s\n", ANSI_COLOR_RESET);

    va_end(args);
}
