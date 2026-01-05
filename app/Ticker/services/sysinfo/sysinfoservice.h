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
    void systemVersionResult(bool success, const QString& versionString);
    
public slots:

    // 实现基类纯虚槽，处理收到的 JSON 消息
    void onMessageReceived(const QJsonDocument& doc) override;

    // 异步获取CPU温度的槽函数
    void onGetCpuTemperature();
    // 异步获取北京时间的槽函数
    void onGetBeijingTime();
    // 异步获取系统镜像版本号的槽函数
    void onGetSystemVersion();

private:

    // 异步获取CPU温度
    void getCpuTemperature();
    // 异步获取北京时间
    void getBeijingTime();
    // 异步获取系统镜像版本号
    void getSystemVersion();

};

#endif // SYSINFOSERVICE_H
