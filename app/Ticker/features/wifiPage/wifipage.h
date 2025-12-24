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

    void Init(void);

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

signals:

    // 向 presenter 发送的信号
    void connectWifiRequested(const QString &ssid, const QString &password);
    void disconnectWifiRequested(void);
    void getWifiStatusRequested(void);

    // 向子页面发送的结果信号
    void getWifiStatusResultToConnSubPage(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);
    void getWifiStatusResultToStaSubPage(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

public slots:

    // 接收 presenter 发送的结果
    void onConnectWifiResult(bool success);
    void onDisconnectWifiResult(bool success);
    void onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

    // 接收子页面发送的请求
    void onGetStatusRequestFromSubPage(void);
    void onDisconnectRequestFromSubPage(void);
    void onConnectRequestFromSubPage(const QString &ssid, const QString &password);

    // 接收 connPage 的切换到 staPage 的请求
    void onSwitchToStaPageRequestFromConnSubPage(void);
    // 接收 staPage 的切换到 connPage 的请求
    void onSwitchToConnPageRequestFromStaSubPage(void);

private:

    // 子页面初始化
    void SubPageInit(void);

    // 切换到指定页面
    void switchToPage(QWidget *target);

private:

    Ui::WifiPage *ui;

    // wifi 连接子页面
    WifiConnPage *m_wifiConnPage;
    // wifi 状态子页面
    WifiStaPage *m_wifiStaPage;

    QWidget *m_lastPageWidget = nullptr;
};

#endif // WIFIPAGE_H
