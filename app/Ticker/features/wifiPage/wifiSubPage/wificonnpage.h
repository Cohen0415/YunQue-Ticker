#ifndef WIFICONNPAGE_H
#define WIFICONNPAGE_H

#include <QWidget>
#include <QTimer>
#include "features/pagelifecycleaware.h"

#define CHECK_WIFI_CONN_INTERVAL_MS    (1000)    // 检测连接状态间隔
#define WIFI_CONN_TIMEOUT_MS           (15000)   // 连接超时

namespace Ui {
class WifiConnPageWidget;
}

class WifiConnPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit WifiConnPage(QWidget *parent = nullptr);
    ~WifiConnPage();

    void init();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;    // 页面进入回调
    void onPageLeave() override;    // 页面离开回调

signals:

    // 向父页面发送的请求信号
    void connectWifiRequested(const QString &ssid, const QString &password);    // 向父页面发送连接 wifi 请求
    void disconnectWifiRequested();     // 向父页面发送断开 wifi 请求
    void getWifiStatusRequested();      // 向父页面发送获取 wifi 状态请求

    void switchToStaPageRequested();    // 向父页面发送切换到状态页面请求

public slots:

    // 接收父页面发送的结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);    // 接收父页面发送的获取 wifi 状态结果

private slots:

    void on_connButton_clicked();       // 连接按钮槽函数
    void on_ssidLineEdit_textChanged(const QString &arg1);  // SSID 输入框内容变化槽函数
    void on_pwdLineEdit_textChanged(const QString &arg1);   // 密码输入框内容变化槽函数
    void on_ssidClearButton_clicked();  // SSID 清除按钮槽函数
    void on_pwdClearButton_clicked();   // 密码清除按钮槽函数
    
    void onStatusTimerTimeout();        // 定时器槽函数

private:

    void uiInit();                              // UI 初始化
    bool inputLineInspect();                    // 用户输入框内容检测

    Ui::WifiConnPageWidget *ui;

    QTimer *m_statusTimer = nullptr;    // 轮询定时器
    int m_elapsed = 0;                  // 已用时ms
    int m_checkInterval;                // 检测间隔ms
    int m_timeoutMs;                    // 超时ms，可调
};

#endif // WIFICONNPAGE_H
