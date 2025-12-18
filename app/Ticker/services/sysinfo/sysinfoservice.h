#ifndef SYSINFOSERVICE_H
#define SYSINFOSERVICE_H

#include "services/abstractservice.h"

class SysinfoService : public AbstractService
{
    Q_OBJECT

public:

    explicit SysinfoService(QObject *parent = nullptr);

    // 实现基类纯虚函数，返回此服务处理的命令列表
    QStringList registeredCommands() const override;
    // 实现基类虚函数，返回服务名称
    QString serviceName() const override;

signals:

    void cpuTemperatureResult(bool success, double temperature);
    void beijingTimeResult(bool success, const QString& timeString);

public slots:

    // 实现基类纯虚槽，处理收到的 JSON 消息
    void onMessageReceived(const QJsonDocument& doc) override;

    // 异步获取CPU温度的槽函数
    void onGetCpuTemperature();
    // 异步获取北京时间的槽函数
    void onGetBeijingTime();

private:

    // 异步获取CPU温度
    void getCpuTemperature();
    // 异步获取北京时间
    void getBeijingTime();

};

#endif // SYSINFOSERVICE_H
