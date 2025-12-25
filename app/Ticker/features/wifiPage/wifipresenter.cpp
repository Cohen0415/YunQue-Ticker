#include "wifipresenter.h"
#include "utils/log/logger.h"

WifiPresenter::WifiPresenter(QObject *parent)
    : QObject{parent}
{}

// 接收到 View 发来的连接 Wi-Fi 请求
void WifiPresenter::onConnectWifiRequested(const QString &ssid, const QString &password)
{
    // 向 Service 发出请求信号
    emit requestConnectWifi(ssid, password);
}

// 接收到 View 发来的断开 Wi-Fi 请求
void WifiPresenter::onDisconnectWifiRequested()
{
    // 向 Service 发出请求信号
    emit requestDisconnectWifi();
}

// 接收到 View 发来的获取 Wi-Fi 状态请求
void WifiPresenter::onGetWifiStatusRequested()
{
    // 向 Service 发出请求信号
    emit requestGetWifiStatus();
}

// 处理来自 Service 的连接 Wi-Fi 结果
void WifiPresenter::handleConnectWifiResult(bool success)
{
    // 将结果通过信号发回给 View
    emit connectWifiResult(success);
}

// 处理来自 Service 的断开连接 Wi-Fi 结果
void WifiPresenter::handleDisconnectWifiResult(bool success)
{
    // 将结果通过信号发回给 View
    emit disconnectWifiResult(success);
}

// 处理来自 Service 的获取 Wi-Fi 状态结果
void WifiPresenter::handleGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    // 将结果通过信号发回给 View
    emit getWifiStatusResult(success, connected, ssid, ip, rssi);
}
