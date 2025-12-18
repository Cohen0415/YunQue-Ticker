#include <QApplication>
#include <QWidget>
#include <QDebug>

#include "widget.h"
#include "utils/log/logger.h"
#include "appcontext.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    installCustomLogger();
    LOG_DEBUG("Application started.");

    AppContext appContext;
    if (appContext.init() != 0)
    {
        LOG_ERROR("Failed to initialize application context.");
        return -1;
    }

    Widget w;
    w.show();
    return app.exec();
}
