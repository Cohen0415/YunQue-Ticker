#include "logger.h"
#include <QDateTime>
#include <QMessageLogContext>
#include <QString>
#include <cstdio>
#include <cstring>

// ANSI 颜色码定义
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// 将 LogLevel 转换为对应的 ANSI 颜色码
const char* logLevelToColorCode(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG: return ANSI_COLOR_CYAN;    // Cyan for Debug
        case LogLevel::INFO:  return ANSI_COLOR_GREEN;   // Green for Info
        case LogLevel::WARN:  return ANSI_COLOR_YELLOW;  // Yellow for Warn
        case LogLevel::ERROR: return ANSI_COLOR_RED;     // Red for Error
        default:              return ANSI_COLOR_RESET;   // Default if unknown
    }
}

// 提取更简洁的函数名
QString extractSimpleFunctionName(const char* fullFunctionSignature)
{
    if (!fullFunctionSignature)
    {
        return QStringLiteral("unknown_func"); // Return a safe placeholder if input is null
    }

    QString signature(fullFunctionSignature);
    if (signature.isEmpty())
    {
        return QStringLiteral("empty_func_sig"); // Return a safe placeholder if input is empty
    }

    // Strategy: Find the opening parenthesis and work backwards.
    int parenIndex = signature.indexOf('(');
    if (parenIndex <= 0)
    {
        // If no '(' or it's the first character, fallback strategy.
        // Often, C++ mangled names or special cases might not have a clear '('.
        // Try to find common patterns like "::function_name".
        int lastColonColonIndex = signature.lastIndexOf("::");
        if (lastColonColonIndex != -1 && lastColonColonIndex + 2 < signature.length())
        {
            // Extract everything after the last "::"
            QString candidate = signature.mid(lastColonColonIndex + 2);
            // If there's still a '(' in the candidate, truncate it.
            int candidateParen = candidate.indexOf('(');
            if (candidateParen != -1)
            {
                return candidate.left(candidateParen);
            }
            else
            {
                // Limit length to prevent overly long names
                if (candidate.length() > 50)
                {
                    return candidate.left(50) + "...";
                }
                return candidate;
            }
        }
        // If all else fails, return a limited portion of the original to avoid clutter.
        if (signature.length() > 50)
        {
            return signature.left(50) + "..._sig";
        }
        return signature;
    }

    // We found a '('. Now try to get the function name part.
    // Look for "::" just before the '('
    int colonColonBeforeParen = signature.lastIndexOf("::", parenIndex);
    if (colonColonBeforeParen != -1 && colonColonBeforeParen + 2 < parenIndex)
    {
        // Found "::", extract the part between "::" and "("
        return signature.mid(colonColonBeforeParen + 2, parenIndex - (colonColonBeforeParen + 2));
    }

    // Look for the last space before the '(' (covers `return_type function_name(` pattern)
    int lastSpaceBeforeParen = signature.lastIndexOf(' ', parenIndex);
    if (lastSpaceBeforeParen != -1 && lastSpaceBeforeParen + 1 < parenIndex)
    {
        // Found a space, extract the part between the space and "("
        return signature.mid(lastSpaceBeforeParen + 1, parenIndex - (lastSpaceBeforeParen + 1));
    }

    // Fallback: Just take the characters from the beginning up to the '('
    // This handles simple C-style functions or cases where parsing failed above.
    return signature.left(parenIndex);
}


// 将 LogLevel 转换为字符串
QString logLevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG: return QStringLiteral("DEBUG");
        case LogLevel::INFO:  return QStringLiteral("INFO ");
        case LogLevel::WARN:  return QStringLiteral("WARN "); // Extra space for alignment
        case LogLevel::ERROR: return QStringLiteral("ERROR");
        default:              return QStringLiteral("UNKN ");
    }
}

// 将 QtMsgType 转换为 LogLevel
LogLevel qtMsgTypeToLogLevel(QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:    return LogLevel::DEBUG;
        case QtInfoMsg:     return LogLevel::INFO;
        case QtWarningMsg:  return LogLevel::WARN;
        case QtCriticalMsg: return LogLevel::ERROR;
        case QtFatalMsg:    return LogLevel::ERROR; // Treat fatal as error
    #if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
        case QtSystemMsg:   return LogLevel::WARN; // Map system messages to warning
    #endif
        default:            return LogLevel::DEBUG; // Default to debug for unknown types
    }
}

// 安装自定义消息处理器
void installCustomLogger()
{
    qInstallMessageHandler(customMessageHandler);
}

// 自定义消息处理函数
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyy-MM-dd hh:mm:ss");

    LogLevel level = qtMsgTypeToLogLevel(type);
    QString levelStr = logLevelToString(level);
    const char* colorCode = logLevelToColorCode(level);
    const char* resetCode = ANSI_COLOR_RESET;

    const char* file = context.file ? context.file : "unknown_file";
    int line = context.line;
    // 函数名提取
    QString simpleFunctionName = extractSimpleFunctionName(context.function);
    // 确保转换后的 QString 数据在 fprintf 期间有效
    QByteArray functionNameByteArray = simpleFunctionName.toLocal8Bit();
    const char* function = functionNameByteArray.constData(); // Use QByteArray's data pointer

    // 格式化字符串：彩色前缀 + 换行缩进的消息
    // Format: COLOR[timestamp] [level] [file:line:function]RESET\n    message\n
    QString formattedMsg = QString("%1[%2] [%3] [%4:%5:%6]%7\n    %8\n")
                               .arg(colorCode)      // Start coloring
                               .arg(timestamp)      // Timestamp
                               .arg(levelStr)       // Log level (colored)
                               .arg(file)           // File name
                               .arg(line)           // Line number
                               .arg(function)       // Simplified function name
                               .arg(resetCode)      // Reset color after prefix
                               .arg(msg);           // Log message (default color)

    QByteArray rawBytes = formattedMsg.toLocal8Bit();
    fprintf(stderr, "%s", rawBytes.constData());
    fflush(stderr);
}
