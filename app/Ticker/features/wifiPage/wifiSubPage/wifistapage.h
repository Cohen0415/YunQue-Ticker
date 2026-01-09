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

    void init();        // 页面初始化，给父页面调用

    // PageLifecycleAware 接口实现
    void onPageEnter() override;    // 页面进入回调
    void onPageLeave() override;    // 页面离开回调

signals:

    // 向父页面发送的信号
    void disconnectWifiRequested();     // 向父页面发送断开 wifi 请求
    void getWifiStatusRequested();      // 向父页面发送获取 wifi 状态请求

    // 向父页面发送切换到状态页面请求信号
    void switchToConnPageRequested();   // 向父页面发送切换到连接页面请求

public slots:

    // 接收父页面发送的结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);    // 接收父页面发送的获取 wifi 状态结果
    void onGetWifiDisconnectResult(bool success);    // 接收父页面发送的断开 wifi 结果

    // 轮询定时器槽函数
    void onStatusTimerTimeout();            // 定时器槽函数

private slots:

    void on_disconnButton_clicked();        // 断开连接按钮槽函数

private:

    void uiInit(void);          // UI 初始化
    WifiRssiLevel getRssiLevel(const int &rssi);    // 获取当前信号等级

    Ui::WifiStaPageWidget *ui;

    WifiRssiLevel m_currentRssiLevel = WIFI_RSSI_LEVEL_0;   // 当前信号等级
    QTimer* m_statusTimer;      // 轮询定时器
    int m_startFlag = 1;        // 首次启动标志
};

#endif // WIFISTAPAGE_H
