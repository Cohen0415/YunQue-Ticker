#ifndef BACKLIGHTSERVICE_H
#define BACKLIGHTSERVICE_H

#include "services/abstractservice.h"


class BacklightService : public AbstractService
{
    Q_OBJECT

public:
    
    explicit BacklightService(QObject *parent = nullptr);

    // 实现基类纯虚函数，返回此服务处理的命令列表。
    QStringList registeredCommands() const override;
    // 实现基类虚函数，返回服务名称。
    QString serviceName() const override;
    // 异步设置亮度值
    void setBrightness(int value);
    // 异步获取当前亮度值，结果通过信号返回
    void getBrightness();

signals:

    void brightnessSetResult(bool success, int newValue);
    void brightnessGetResult(bool success, int currentValue);

public slots:

    // 实现基类纯虚槽，处理收到的 JSON 消息。
    void onMessageReceived(const QJsonDocument& doc) override;

private:
    
    static const int MIN_BRIGHTNESS = 0;
    static const int MAX_BRIGHTNESS = 255;
};

#endif // BACKLIGHTSERVICE_H