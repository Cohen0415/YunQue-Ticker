#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include "services/abstractservice.h"

#define MIN_VOLUME_LOGIC        (0)       // 逻辑音量最小值
#define MAX_VOLUME_LOGIC        (100)     // 逻辑音量最大值

class AudioService : public AbstractService
{
    Q_OBJECT

public:

    explicit AudioService(QObject *parent = nullptr);

    // 实现基类纯虚函数，返回此服务处理的命令列表
    QStringList registeredCommands() const override;
    // 实现基类虚函数，返回服务名称
    QString serviceName() const override;

signals:

    void volumeSetResult(bool success, int newValue);
    void volumeGetResult(bool success, int currentValue);
    void audioPlayResult(bool success);
    void audioStopResult(bool success);

public slots:

    // 实现基类纯虚槽，处理收到的 JSON 消息
    void onMessageReceived(const QJsonDocument& doc) override;

    // 异步设置音量值的槽函数
    void onSetVolume(int value);
    // 异步获取当前音量值的槽函数
    void onGetVolume();
    // 异步播放音频文件的槽函数
    void onPlayAudioFile(const QString& filePath);
    // 异步停止音频播放的槽函数
    void onStopAudio();

private:

    // 异步设置音量值
    void setVolume(int value);
    // 获取当前音量值
    void getVolume();
    // 异步播放音频文件
    void playAudioFile(const QString& filePath);
    // 异步停止音频播放
    void stopAudio();

};

#endif // AUDIOSERVICE_H
