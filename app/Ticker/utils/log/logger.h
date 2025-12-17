#ifndef LOGGER_H
#define LOGGER_H

#include <QtGlobal>
#include <QString>
#include <QDebug>

// 日志级别枚举 (从 0 开始)
enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

// 安装自定义消息处理器
void installCustomLogger();

// 自定义消息处理函数 (内部使用)
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

// 辅助函数 (内部使用)
LogLevel qtMsgTypeToLogLevel(QtMsgType type);
QString logLevelToString(LogLevel level);

// 方便使用的宏定义
#define LOG_DEBUG(...) \
    do { \
        qDebug(__VA_ARGS__); \
    } while (0)

#define LOG_INFO(...) \
    do { \
        qInfo(__VA_ARGS__); \
    } while (0)

#define LOG_WARN(...) \
    do { \
        qWarning(__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(...) \
    do { \
        qCritical(__VA_ARGS__); \
    } while (0)

#endif // LOGGER_H
