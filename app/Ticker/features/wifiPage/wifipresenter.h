#ifndef WIFIPRESENTER_H
#define WIFIPRESENTER_H

#include <QObject>

class WifiPresenter : public QObject
{
    Q_OBJECT

public:

    explicit WifiPresenter(QObject *parent = nullptr);

signals:

    // ===== 向 Service 请求 =====
    void requestConnectWifi(const QString &ssid, const QString &password);  // 向 service 发送连接 wifi 请求
    void requestDisconnectWifi(void);                                       // 向 service 发送断开 wifi 请求  
    void requestGetWifiStatus(void);                                        // 向 service 发送获取 wifi 状态请求    

    // ===== 向 View/Model 发结果 =====
    void connectWifiResult(bool success);           // 向 view 发送连接 wifi 结果
    void disconnectWifiResult(bool success);        // 向 view 发送断开 wifi 结果
    void getWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);  // 向 view 发送获取 wifi 状态结果

public slots:

    // ===== 接收 View 的请求 =====
    void onConnectWifiRequested(const QString &ssid, const QString &password);  // 接收 view 发送的连接 wifi 请求
    void onDisconnectWifiRequested(void);           // 接收 view 发送的断开 wifi 请求
    void onGetWifiStatusRequested(void);            // 接收 view 发送的获取 wifi 状态请求

    // ===== 接收 Service 的返回 =====
    void handleConnectWifiResult(bool success);     // 接收 service 连接 wifi 结果
    void handleDisconnectWifiResult(bool success);  // 接收 service 断开 wifi 结果
    void handleGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);  // 接收 service 获取 wifi 状态结果

private slots:

private:

};

#endif // WIFIPRESENTER_H
