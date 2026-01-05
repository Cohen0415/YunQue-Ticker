#include "sysinfoservice.h"
#include "utils/log/logger.h"
#include <QJsonObject>
#include <QJsonValue>

SysinfoService::SysinfoService(QObject *parent)
    : AbstractService(parent)
{
    // LOG_DEBUG("SysinfoService Constructor called.");
}

QStringList SysinfoService::registeredCommands() const
{
    return QStringList()
           << "sysinfo.bjtime.get"
           << "sysinfo.temp.get"
           << "sysinfo.version.get";
}

QString SysinfoService::serviceName() const
{
    return QStringLiteral("SysinfoService");
}

void SysinfoService::onMessageReceived(const QJsonDocument& doc)
{
    if (!doc.isObject()) 
    {
        LOG_WARN("Received JSON data is not an object.");
        return;
    }

    QJsonObject responseObj = doc.object();
    QString command = responseObj["cmd"].toString();
    int status = responseObj["status"].toInt(-1);
    QString msg = responseObj["msg"].toString("No message provided");
    QJsonValue dataValue = responseObj["data"];

    bool success = (status == 0);

    if (command == "sysinfo.temp.get") 
    {
        /*
            {
                "cmd": "sysinfo.temp.get",
                "status": 0,
                "msg": "ok",
                "data": {
                    "temp": 45493   // 除1000就是摄氏度
                }
            }
        */
        int temperature = -1;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["temp"];
            if (valueInData.isDouble()) 
            {
                temperature = static_cast<int>(valueInData.toDouble());
            } 
            else 
            {
                LOG_WARN("'sysinfo.temp.get' response 'data.temperature' is missing or not a number.");
                success = false; // 即使 status=0, 数据不对也认为失败
            }   
        }
        else if (!success) 
        {
            LOG_WARN("'sysinfo.temp.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'sysinfo.temp.get' response 'data' is missing or not an object.");
            success = false;
        }

        // 发射信号通知调用者结果
        emit cpuTemperatureResult(success, temperature*1.0/1000);
    } 
    else if (command == "sysinfo.bjtime.get") 
    {
        /*
            {
                "cmd": "sysinfo.bjtime.get",
                "status": 0,
                "msg": "ok",
                "data": {
                    "time": "2025-12-05T09:26:25+08:00"
                }
            }
        */
        QString timeString;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["time"];
            if (valueInData.isString()) 
            {
                timeString = valueInData.toString();  
            }
            else 
            {
                LOG_WARN("'sysinfo.bjtime.get' response 'data.time' is missing or not a string.");
                success = false; // 即使 status=0, 数据不对也认为失败
            }
        }
        else if (!success) 
        {
            LOG_WARN("'sysinfo.bjtime.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {       
            LOG_WARN("'sysinfo.bjtime.get' response 'data' is missing or not an object.");
            success = false;
        }
        
        // 发射信号通知调用者结果
        emit beijingTimeResult(success, timeString);;
    } 
    else if (command == "sysinfo.version.get")
    {
        /*
            {
                "cmd": "sysinfo.version.get",
                "status": 0,
                "msg": "ok",
                "data": {
                    "version": "v1.0.0"
                }
            }
        */
        QString versionString;
        if (success && dataValue.isObject())
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["version"];
            if (valueInData.isString())
            {
                versionString = valueInData.toString();
            }
            else
            {
                LOG_WARN("'sysinfo.version.get' response 'data.version' is missing or not a string.");
                success = false; // 即使 status=0, 数据不对也认为失败
            }
        }
        else if (!success)
        {
            LOG_WARN("'sysinfo.version.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        }
        else
        {
            LOG_WARN("'sysinfo.version.get' response 'data' is missing or not an object.");
            success = false;
        }

        // 这里可以发射一个信号通知调用者结果，假设有相应的信号定义
        emit systemVersionResult(success, versionString);
    }
    else 
    {
        LOG_WARN("Received unknown command response: %s", command.toLocal8Bit().constData());
    }
}

void SysinfoService::getCpuTemperature()
{
    /*
        {
            "cmd": "sysinfo.temp.get",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("sysinfo.temp.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void SysinfoService::getBeijingTime()
{
    /*
        {
            "cmd": "sysinfo.bjtime.get",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("sysinfo.bjtime.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void SysinfoService::getSystemVersion()
{
    /*
        {
            "cmd": "sysinfo.version.get",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("sysinfo.version.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void SysinfoService::onGetCpuTemperature()
{
    getCpuTemperature();
}

void SysinfoService::onGetBeijingTime()
{
    getBeijingTime();
}

void SysinfoService::onGetSystemVersion()
{
    getSystemVersion();
}