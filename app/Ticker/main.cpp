#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>
#include <QtGlobal>

#include "widget.h"
#include "utils/log/logger.h"
#include "appcontext.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;

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

    w.show();
    return app.exec();
}
