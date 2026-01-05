#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>
#include <QtGlobal>
#include <QFile>

#include "widget.h"
#include "utils/log/logger.h"
#include "appcontext.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;

    // 打印版本号
    LOG_DEBUG("App Version: %s", APP_GIT_VERSION);

    // 打印系统运行至今的时间，以此判断设备从上电到进入桌面花了多长时间
    QFile file("/proc/uptime");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray line = file.readLine();
        QList<QByteArray> parts = line.split(' ');
        if (parts.size() >= 1)
        {
            double uptimeSeconds = parts[0].toDouble();
            LOG_DEBUG("System Uptime : %d s", uptimeSeconds);
        }
        file.close();
    }

    // 1、安装自定义日志系统
    installCustomLogger();

    // 2、设置字体
    int id = QFontDatabase::addApplicationFont(":/res/font/AlimamaShuHeiTi-Bold.ttf");
    if (id != -1)
    {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont font(family);
        app.setFont(font);
    }
    else
    {
        LOG_DEBUG("Faild to load custom font.");
    }

    // 3、初始化 AppContext
#if defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)

#else
    if (AppContext::getInstance()->init() != 0)
    {
        LOG_ERROR("Failed to initialize application context.");
        return -1;
    }
#endif

    // 4、初始化 Widget
    w.init();

    w.show();
    return app.exec();
}
