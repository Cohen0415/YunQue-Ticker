#ifndef PAGEMSGMANAGER_H
#define PAGEMSGMANAGER_H

#include <QObject>

class PageMsgManager : public QObject
{
    Q_OBJECT

public:

    explicit PageMsgManager(QObject *parent = nullptr);

    static PageMsgManager *getInstance();

signals:

    // 定义页面间通信的信号
    // 音量静音状态改变
    void volumeMuteStateChanged(bool isMuted);

private:

    static PageMsgManager *m_instance;

};

#endif // PAGEMSGMANAGER_H
