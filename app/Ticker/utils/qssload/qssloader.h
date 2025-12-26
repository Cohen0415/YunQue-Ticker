#ifndef QSSLOADER_H
#define QSSLOADER_H

#include <QObject>

class QssLoader : public QObject
{
    Q_OBJECT

public:

    explicit QssLoader(QObject *parent = nullptr);

    static QString load(const QString &path);

signals:

};

#endif // QSSLOADER_H
