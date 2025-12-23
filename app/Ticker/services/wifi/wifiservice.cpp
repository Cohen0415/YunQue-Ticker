#include "wifiservice.h"
#include "utils/log/logger.h"
#include <QJsonObject>
#include <QJsonValue>

WifiService::WifiService(QObject *parent)
    : AbstractService(parent)
{
    // LOG_DEBUG("WifiService Constructor called.");
}

QStringList WifiService::registeredCommands() const
{
    return QStringList()
           << "wifi.connect"
           << "wifi.disconnect"
           << "wifi.status.get";
}

QString WifiService::serviceName() const
{
    return QStringLiteral("WifiService");
}

void WifiService::connectToNetwork(const QString& ssid, const QString& password)
{
    /*
        {
            "cmd": "wifi.connect",
            "params": {
                "ssid": "Home_Wifi",
                "password": "12345678"
            }
        }
    */

    if (ssid.isEmpty()) 
    {
        LOG_WARN("connectToNetwork called with empty ssid.");
        return;
    }

    QJsonObject request;
    request["cmd"] = QStringLiteral("wifi.connect");

    QJsonObject params;
    params["ssid"] = ssid;
    params["psw"] = password;
    request["params"] = params;

    sendMessage(QJsonDocument(request));
}

void WifiService::disconnectFromNetwork()
{
    /*
        {
            "cmd": "wifi.disconnect",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("wifi.disconnect");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void WifiService::getWifiStatus()
{
    /*
        {
            "cmd": "wifi.status.get",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("wifi.status.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void WifiService::onMessageReceived(const QJsonDocument& doc)
{
    if (!doc.isObject()) 
    {
        LOG_WARN("Received JSON data is not an object.");
        return;
    }

    QJsonObject responseObj = doc.object();
    QString command = responseObj["cmd"].toString();

    if (command.isEmpty()) 
    {
        LOG_WARN("Received response without 'cmd' field.");
        return;
    }

    int status = responseObj["status"].toInt(-1); // 默认 -1 表示解析失败或不存在
    bool success = (status == 0);
    QString msg = responseObj["msg"].toString("No message provided");
    QJsonValue dataValue = responseObj["data"];

    // 这里可以根据 command 处理不同的响应消息
    if (command == "wifi.connect")
    {
        /*
            {
                "cmd": "wifi.connect",
                "status": 0,
                "msg": "ok",
                "data": {
                    "ip": "192.168.1.100"
                }
            }
        */

        if (!success) 
        {
            LOG_WARN("'wifi.connect' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        }

        emit connectResult(success);
    }
    else if (command == "wifi.disconnect")
    {
        /*
            {
                "cmd": "wifi.disconnect",
                "status": 0,
                "msg": "ok"
            }
        */
        
        if (!success) 
        {
            LOG_WARN("'wifi.disconnect' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        }

        emit disconnectResult(success);
    }
    else if (command == "wifi.status.get")
    {
        /*
            {
                "cmd": "wifi.status.get",
                "status": 0,
                "msg": "ok",
                "data": {
                    "connected": true,
                    "ssid": "Home_Wifi",
                    "ip": "192.168.1.100",
                    "rssi": "-50"
                }
            }
        */

        WifiNetworkInfo info;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            info.connected = dataObj["connected"].toBool(false);
            info.ssid = dataObj["ssid"].toString();
            info.ip = dataObj["ip"].toString();
            info.rssi = dataObj["rssi"].toString();
        }
        else if (!success) 
        {
            LOG_WARN("'wifi.status.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'wifi.status.get' response 'data' is missing or not an object.");
            success = false;
        }

        emit wifiStatusResult(success, info.connected, info.ssid, info.ip, info.rssi);
    }
    else 
    {
        LOG_WARN("Received response for unknown command: %s", command.toLocal8Bit().constData());
    }
}

void WifiService::onConnectToNetwork(const QString &ssid, const QString &password)
{
    connectToNetwork(ssid, password);
}

void WifiService::onDisconnectFromNetwork()
{
    disconnectFromNetwork();
}

void WifiService::onGetWifiStatus()
{
    getWifiStatus();
}
