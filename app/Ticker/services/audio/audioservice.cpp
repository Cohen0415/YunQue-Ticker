#include "audioservice.h"
#include "utils/log/logger.h"
#include <QJsonObject>
#include <QJsonValue>

AudioService::AudioService(QObject *parent)
    : AbstractService(parent)
{
    // LOG_DEBUG("AudioService Constructor called.");
}

QStringList AudioService::registeredCommands() const
{
    return QStringList()
           << "audio.play"
           << "audio.stop"
           << "audio.volume.set"
           << "audio.volume.get";
}

QString AudioService::serviceName() const
{
    return QStringLiteral("AudioService");
}

void AudioService::setVolume(int value)
{
    /*
        {
            "cmd": "audio.volume.set",
            "params": {
                "volume": 200
            }
        }
    */

    if (value < MIN_VOLUME_LOGIC || value > MAX_VOLUME_LOGIC) 
    {
        LOG_WARN("setVolume called with invalid value %d. Must be between %d and %d.",
                 value, MIN_VOLUME_LOGIC, MAX_VOLUME_LOGIC);
        return;
    }

    QJsonObject request;
    request["cmd"] = QStringLiteral("audio.volume.set");

    QJsonObject params;
    params["volume"] = value;
    request["params"] = params;

    sendMessage(QJsonDocument(request));
}

void AudioService::getVolume()
{
    /*
        {
            "cmd": "audio.volume.get",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("audio.volume.get");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void AudioService::playAudioFile(const QString& filePath)
{
    /*
        {
            "cmd": "audio.play",
            "params": {
                "name": "happy.wav"
            }
        }
    */

    // 检查文件是否存在
    if (filePath.isEmpty()) 
    {
        LOG_WARN("playAudioFile called with empty filePath.");
        return;
    }

    QJsonObject request;
    request["cmd"] = QStringLiteral("audio.play");
    QJsonObject params;
    params["name"] = filePath;
    request["params"] = params;

    sendMessage(QJsonDocument(request));
}

void AudioService::stopAudio()
{
    /*
        {
            "cmd": "audio.stop",
            "params": {}
        }
    */

    QJsonObject request;
    request["cmd"] = QStringLiteral("audio.stop");
    request["params"] = QJsonObject(); // 空的 params 对象

    sendMessage(QJsonDocument(request));
}

void AudioService::onMessageReceived(const QJsonDocument& doc)
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

    if (command == "audio.volume.set") 
    {
        /*
            {
                "cmd": "audio.volume.set",
                "status": 0,
                "msg": "ok",
                "data": {
                    "volume": 200
                }
            }
        */
        int resultValue = -1;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["volume"];
            if (valueInData.isDouble()) 
            {
                resultValue = static_cast<int>(valueInData.toDouble());
            } 
            else 
            {
                LOG_WARN("'audio.volume.set' response 'data.volume' is missing or not a number.");
                success = false; // 即使 status=0, 数据不对也认为失败
            }   
        }
        else if (!success) 
        {
            LOG_WARN("'audio.volume.set' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'audio.volume.set' response 'data' is missing or not an object.");
            success = false;
        }

        // 发射信号通知调用者结果
        emit volumeSetResult(success, resultValue);
    } 
    else if (command == "audio.volume.get") 
    {
        /*
            {
                "cmd": "audio.volume.get",
                "status": 0,
                "msg": "ok",
                "data": {
                    "volume": 200
                }
            }
        */
        int resultValue = -1;
        if (success && dataValue.isObject()) 
        {
            QJsonObject dataObj = dataValue.toObject();
            QJsonValue valueInData = dataObj["volume"];
            if (valueInData.isDouble()) 
            {
                resultValue = static_cast<int>(valueInData.toDouble());
            }
            else 
            {
                LOG_WARN("'audio.volume.get' response 'data.volume' is missing or not a number.");
                success = false;
            }
        }
        else if (!success) 
        {
            LOG_WARN("'audio.volume.get' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        } 
        else 
        {
            LOG_WARN("'audio.volume.get' response 'data' is missing or not an object.");
            success = false;
        }

        // 发射信号通知调用者结果
        emit volumeGetResult(success, resultValue);
    }
    else if (command == "audio.play") 
    {
        /*
            {
                "cmd": "audio.play",
                "status": 0,
                "msg": "ok",
                "data": {}
            }
        */
        if (!success) 
        {
            LOG_WARN("'audio.play' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        }

        emit audioPlayResult(success);
    } 
    else if (command == "audio.stop") 
    {
        /*
            {
                "cmd": "audio.stop",
                "status": 0,
                "msg": "ok",
                "data": {}
            }
        */
        if (!success) 
        {
            LOG_WARN("'audio.stop' failed on server. Status: %d Msg: %s", status, msg.toLocal8Bit().constData());
        }

        emit audioStopResult(success);
    } 
    else 
    {
        LOG_WARN("Received response for unknown command: %s", command.toLocal8Bit().constData());
    }
}

void AudioService::onSetVolume(int value)
{
    setVolume(value);
}

void AudioService::onGetVolume()
{
    getVolume();
}

void AudioService::onPlayAudioFile(const QString &filePath)
{
    playAudioFile(filePath);
}

void AudioService::onStopAudio()
{
    stopAudio();
}
