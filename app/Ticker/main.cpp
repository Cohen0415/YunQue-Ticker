#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>
#include <QtGlobal>
#include <QFile>
#include <QRegularExpression>

#include "widget.h"
#include "utils/log/logger.h"
#include "appcontext.h"

static QString normalizeVersion(const QString &gitDesc)
{
    // 匹配：v0.5.0-9-gxxxx
    static QRegularExpression re(
        R"(^(v\d+\.\d+)\.\d+-(\d+)-g[0-9a-f]+)"
    );

    QRegularExpressionMatch m = re.match(gitDesc);
    if (!m.hasMatch())
        return gitDesc;   

    QString base = m.captured(1);   
    QString dist = m.captured(2);   

    return base + "." + dist;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;

    // 先初始化 UI，快速显示桌面
    w.init0();
    w.show();

    QTimer::singleShot(0, [&]() {

        // 安装自定义日志系统
        installCustomLogger();
        g_logLevel = LogLevel::INFO; // 设置日志等级

        QString normalizedVersion = normalizeVersion(APP_GIT_VERSION);
        LOG_INFO("Application Version: %s", normalizedVersion.toStdString().c_str());
        LOG_INFO("Starting application initialization...");

        // 初始化 AppContext
#if defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)

#else
        if (AppContext::getInstance()->init() != 0)
        {
            LOG_ERROR("Failed to initialize application context.");
            return -1;
        }
#endif

        // 初始化 Widget
        w.init();
        LOG_INFO("Widget initialized.");

        // 设置字体
        int id = QFontDatabase::addApplicationFont(":/res/font/AlimamaShuHeiTi-Bold.ttf");
        if (id != -1)
        {
            QString family = QFontDatabase::applicationFontFamilies(id).at(0);
            QFont font(family);
            app.setFont(font);
        }
        else
        {
            LOG_WARN("Failed to load custom font.");
        }
        LOG_INFO("Application font set.");

        LOG_INFO("Application initialization completed.");
    });

    return app.exec();
}
