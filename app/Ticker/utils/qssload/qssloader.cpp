#include "qssloader.h"
#include "utils/log/logger.h"

#include <QFile>
#include <QTextStream>

QssLoader::QssLoader(QObject *parent)
    : QObject{parent}
{}

QString QssLoader::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return QString(); // 打开失败返回空字符串

    QTextStream in(&file);
    in.setCodec("UTF-8"); // 保证中文正常显示

    QString style = in.readAll();
    file.close();

    LOG_DEBUG("QSS loaded from %s, size: %d bytes",
              path.toStdString().c_str(),
              style.toUtf8().size());
    return style;
}
