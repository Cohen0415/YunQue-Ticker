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
    void volumeMuteStateChanged(bool isMuted);  // 音量静音状态改变
    void wifiStatusChanged(bool connected);     // wifi 状态改变
    void bjTimeUpdated(const QString &bjTime);  // 北京时间更新

private:

    static PageMsgManager *m_instance;

};

#endif // PAGEMSGMANAGER_H
