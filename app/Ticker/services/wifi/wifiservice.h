#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include "services/abstractservice.h"

typedef struct {
    bool connected;
    QString ssid;
    QString ip;
    QString rssi;
} WifiNetworkInfo;

class WifiService : public AbstractService
{
    Q_OBJECT

public:
    
    explicit WifiService(QObject *parent = nullptr);

    // 实现基类纯虚函数，返回此服务处理的命令列表
    QStringList registeredCommands() const override;
    // 实现基类虚函数，返回服务名称
    QString serviceName() const override;

signals:

    void connectResult(bool success);
    void disconnectResult(bool success);
    void wifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi);

public slots:

    // 实现基类纯虚槽，处理收到的 JSON 消息
    void onMessageReceived(const QJsonDocument& doc) override;

    // 异步连接到指定WiFi网络的槽函数
    void onConnectToNetwork(const QString& ssid, const QString& password);
    // 异步断开当前WiFi连接的槽函数
    void onDisconnectFromNetwork();
    // 异步获取当前WiFi状态的槽函数
    void onGetWifiStatus();

private:

    // 异步连接到指定WiFi网络
    void connectToNetwork(const QString& ssid, const QString& password);
    // 异步断开当前WiFi连接
    void disconnectFromNetwork();
    // 异步获取当前WiFi状态
    void getWifiStatus();

};

#endif // WIFISERVICE_H
