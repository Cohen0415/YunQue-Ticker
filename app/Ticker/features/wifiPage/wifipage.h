#ifndef WIFIPAGE_H
#define WIFIPAGE_H

#include <QWidget>
#include "features/pagelifecycleaware.h"
#include "features/wifiPage/wifiSubPage/wifistapage.h"
#include "features/wifiPage/wifiSubPage/wificonnpage.h"

namespace Ui {
class WifiPage;
}

class WifiPage : public QWidget , public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit WifiPage(QWidget *parent = nullptr);
    ~WifiPage();

    // 页面初始化，给 widget.cpp 调用
    void init();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;        // 页面进入回调
    void onPageLeave() override;        // 页面离开回调

signals:

    // 向 presenter 发送的信号
    void connectWifiRequested(const QString &ssid, const QString &password);    // 向 presenter 发送连接 wifi 请求
    void disconnectWifiRequested();                                         // 向 presenter 发送断开 wifi 请求            
    void getWifiStatusRequested();                                          // 向 presenter 发送获取 wifi 状态请求

    // 向子页面发送的结果信号
    void getWifiStatusResultToConnSubPage(bool success, bool connected, QString &ssid, QString &ip, QString &rssi); // 发送 wifi 状态给 conn 子页面
    void getWifiStatusResultToStaSubPage(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);  // 发送 wifi 状态给 sta 子页面
    void getWifiDisconnectResultToStaSubPage(bool success);    // 发送断开 wifi 结果给 sta 子页面

public slots:

    // 接收 presenter 发送的结果
    void onConnectWifiResult(bool success);         // 接收 presenter 发送的连接 wifi 结果
    void onDisconnectWifiResult(bool success);      // 接收 presenter 发送的断开 wifi 结果
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);    // 接收 presenter 发送的获取 wifi 状态结果

    // 接收子页面发送的请求
    void onGetStatusRequestFromSubPage();       // 接收子页面发送的获取 wifi 状态请求
    void onDisconnectRequestFromSubPage();      // 接收子页面发送的断开 wifi 请求
    void onConnectRequestFromSubPage(const QString &ssid, const QString &password); // 接收子页面发送的连接 wifi 请求

    // 接收 connPage 的切换到 staPage 的请求
    void onSwitchToStaPageRequestFromConnSubPage();     // 接收 connPage 的切换到 staPage 的请求
    // 接收 staPage 的切换到 connPage 的请求
    void onSwitchToConnPageRequestFromStaSubPage();     // 接收 staPage 的切换到 connPage 的请求

private:

    void subPageInit();                 // 子页面初始化
    void switchToPage(QWidget *target); // 切换到指定页面

    Ui::WifiPage *ui;

    WifiConnPage *m_wifiConnPage;       // wifi 连接子页面
    WifiStaPage *m_wifiStaPage;         // wifi 状态子页面

    QWidget *m_lastPageWidget = nullptr;    // 记录上一个页面指针
};

#endif // WIFIPAGE_H
