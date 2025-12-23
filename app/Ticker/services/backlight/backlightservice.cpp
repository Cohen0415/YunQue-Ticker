#include "backlightservice.h"
#include "utils/log/logger.h"
#include <QJsonObject>
#include <QJsonValue>

BacklightService::BacklightService(QObject *parent)
    : AbstractService(parent)
{
    // LOG_DEBUG("BacklightService Constructor called.");
}

QStringList BacklightService::registeredCommands() const
{
    // 注册命令列表
    return QStringList({"brightness.set", "brightness.get"});
}

QString BacklightService::serviceName() const
{
    return QStringLiteral("BacklightService");
}

void BacklightService::setBrightness(int value)
{
    if (value < MIN_BRIGHTNESS_LOGIC || value > MAX_BRIGHTNESS_LOGIC) 
    {
        LOG_WARN("setBrightness called with invalid value %d. Must be between %d and %d.",
                 value, MIN_BRIGHTNESS_LOGIC, MAX_BRIGHTNESS_LOGIC);
        return;
    }

    // 构造 'brightness.set' 请求
    QJsonObject request;
    request["cmd"] = QStringLiteral("brightness.set");

    QJsonObject params;
    params["value"] = value;
    request["params"] = params;

    // LOG_DEBUG("[" + serviceName() + "] Sending 'brightness.set' request with value: %d.", value);
    // 通过基类的 sendMessage 将请求发送出去
    sendMessage(QJsonDocument(request));
}

void BacklightService::getBrightness()
{
    // 构造 'brightness.get' 请求
    QJsonObject request;
    request["cmd"] = QStringLiteral("brightness.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    // ("[" + serviceName() + "] Sending 'brightness.get' request.");
    // 通过基类的 sendMessage 将请求发送出去
    sendMessage(QJsonDocument(request));
}

void BacklightService::onMessageReceived(const QJsonDocument& doc)
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

    // LOG_DEBUG("Received response for command: %s", command);

    int status = responseObj["status"].toInt(-1); // 默认 -1 表示解析失败或不存在
    bool success = (status == 0);
    QString msg = responseObj["msg"].toString("No message provided");
    QJsonValue dataValue = responseObj["data"];

    if (command == "brightness.set") 
    {
        int resultValue = -1;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["value"];
            if (valueInData.isDouble()) 
            {
                resultValue = static_cast<int>(valueInData.toDouble());
            } 
            else 
            {
                LOG_WARN("'brightness.set' response 'data.value' is missing or not a number.");
                success = false; // 即使 status=0, 数据不对也认为失败
            }
        } 
        else if (!success) 
        {
            LOG_WARN("'brightness.set' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'brightness.set' response 'data' is missing or not an object.");
            success = false;
        }
        
        // 发射信号通知调用者结果
        emit brightnessSetResult(success, resultValue);

    } 
    else if (command == "brightness.get") 
    {
        int resultValue = -1;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["value"];
            if (valueInData.isDouble()) 
            {
                resultValue = static_cast<int>(valueInData.toDouble());
            } 
            else 
            {
                LOG_WARN("'brightness.get' response 'data.value' is missing or not a number.");
                success = false;
            }
        } 
        else if (!success) 
        {
            LOG_WARN("'brightness.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'brightness.get' response 'data' is missing or not an object.");
            success = false;
        }

        // 发射信号通知调用者结果
        emit brightnessGetResult(success, resultValue);
    } 
    else 
    {
        LOG_WARN("Received response for unknown command: %s", command.toLocal8Bit().constData());
    }
}

void BacklightService::onSetBrightness(int value)
{
    setBrightness(value);
}

void BacklightService::onGetBrightness()
{
    getBrightness();
}
