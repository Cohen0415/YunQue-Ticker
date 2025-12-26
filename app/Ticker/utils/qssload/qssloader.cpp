#include "qssloader.h"
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
    return style;
}
