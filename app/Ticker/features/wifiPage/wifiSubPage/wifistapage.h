#ifndef WIFISTAPAGE_H
#define WIFISTAPAGE_H

#include <QWidget>
#include <QTimer>
#include "features/pagelifecycleaware.h"

#define REFRESH_WIFI_STA_MS     2000    // 定时器轮询间隔

enum WifiRssiLevel {
    WIFI_RSSI_LEVEL_0 = 0,
    WIFI_RSSI_LEVEL_1,
    WIFI_RSSI_LEVEL_2,
    WIFI_RSSI_LEVEL_3,
    WIFI_RSSI_LEVEL_4
};

namespace Ui {
class WifiStaPageWidget;
}

class WifiStaPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit WifiStaPage(QWidget *parent = nullptr);
    ~WifiStaPage();

    void Init();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

signals:

    // 向父页面发送的信号
    void disconnectWifiRequested(void);
    void getWifiStatusRequested(void);

    // 向父页面发送切换到状态页面请求信号
    void switchToConnPageRequested(void);

public slots:

    // 接收父页面发送的结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

    // 轮询定时器槽函数
    void onStatusTimerTimeout();

private slots:

    void on_disconnButton_clicked();

private:

    // UI 初始化
    void UIInit(void);

    // 获取当前信号等级
    WifiRssiLevel getRssiLevel(const int &rssi);

    // 加载样式表
    QString LoadQssStyle(const QString &path);

private:

    Ui::WifiStaPageWidget *ui;

    WifiRssiLevel m_currentRssiLevel = WIFI_RSSI_LEVEL_0;   // 当前信号等级

    QTimer* m_statusTimer;      // 轮询定时器

    int m_startFlag = 1;        // 首次启动标志
};

#endif // WIFISTAPAGE_H
