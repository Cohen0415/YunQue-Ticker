#include "backlightservice.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

BacklightService::BacklightService(QObject *parent)
    : AbstractService(parent)
{
    qDebug() << "[" << serviceName() << "] Initialized.";
}

QStringList BacklightService::registeredCommands() const
{
    // 声明此服务可以处理这两个命令的响应
    return QStringList({"brightness.set", "brightness.get"});
}

QString BacklightService::serviceName() const
{
    return QStringLiteral("BacklightService");
}

void BacklightService::setBrightness(int value)
{
    if (value < MIN_BRIGHTNESS || value > MAX_BRIGHTNESS) 
    {
        qWarning() << "[" << serviceName() << "] setBrightness called with invalid value:" << value;
        // 可以立即发射失败信号，或者不发送请求直接失败
        // emit brightnessSetResult(false, -1);
        return;
    }

    // 构造 'brightness.set' 请求
    QJsonObject request;
    request["cmd"] = QStringLiteral("brightness.set");

    QJsonObject params;
    params["value"] = value;
    request["params"] = params;

    qDebug() << "[" << serviceName() << "] Sending 'brightness.set' request with value:" << value;
    // 通过基类的 sendMessage 将请求发送出去
    sendMessage(QJsonDocument(request));
}

void BacklightService::getBrightness()
{
    // 构造 'brightness.get' 请求
    QJsonObject request;
    request["cmd"] = QStringLiteral("brightness.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    qDebug() << "[" << serviceName() << "] Sending 'brightness.get' request.";
    // 通过基类的 sendMessage 将请求发送出去
    sendMessage(QJsonDocument(request));
}

void BacklightService::onMessageReceived(const QJsonDocument& doc)
{
    if (!doc.isObject()) 
    {
        qWarning() << "[" << serviceName() << "] Received non-object JSON document as response.";
        return;
    }

    QJsonObject responseObj = doc.object();
    QString command = responseObj["cmd"].toString();

    if (command.isEmpty()) 
    {
        qWarning() << "[" << serviceName() << "] Received response JSON lacks 'cmd' field.";
        return;
    }

    qDebug() << "[" << serviceName() << "] Processing server response for command:" << command;

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
                qWarning() << "[" << serviceName() << "] 'brightness.set' response 'data.value' is missing or not a number.";
                success = false; // 即使 status=0, 数据不对也认为失败
            }
        } 
        else if (!success) 
        {
             qWarning() << "[" << serviceName() << "] 'brightness.set' failed on server. Status:" << status << "Msg:" << msg;
        } 
        else 
        {
            qWarning() << "[" << serviceName() << "] 'brightness.set' response 'data' is missing or not an object.";
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
                qWarning() << "[" << serviceName() << "] 'brightness.get' response 'data.value' is missing or not a number.";
                success = false;
            }
        } 
        else if (!success) 
        {
             qWarning() << "[" << serviceName() << "] 'brightness.get' failed on server. Status:" << status << "Msg:" << msg;
        } 
        else 
        {
            qWarning() << "[" << serviceName() << "] 'brightness.get' response 'data' is missing or not an object.";
            success = false;
        }

        // 发射信号通知调用者结果
        emit brightnessGetResult(success, resultValue);

    } 
    else 
    {
        qWarning() << "[" << serviceName() << "] Received response for unexpected command:" << command;
    }
}