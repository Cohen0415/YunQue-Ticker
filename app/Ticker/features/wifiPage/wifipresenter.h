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
    void requestConnectWifi(const QString &ssid, const QString &password);
    void requestDisconnectWifi(void);
    void requestGetWifiStatus(void);

    // ===== 向 View/Model 发结果 =====
    void connectWifiResult(bool success);
    void disconnectWifiResult(bool success);
    void getWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

public slots:

    // ===== 接收 View 的请求 =====
    void onConnectWifiRequested(const QString &ssid, const QString &password);
    void onDisconnectWifiRequested(void);
    void onGetWifiStatusRequested(void);

    // ===== 接收 Service 的返回 =====
    void handleConnectWifiResult(bool success);
    void handleDisconnectWifiResult(bool success);
    void handleGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

};

#endif // WIFIPRESENTER_H
