#ifndef ABSTRACTSERVICE_H
#define ABSTRACTSERVICE_H

#include <QObject>
#include <QJsonDocument>
#include <QStringList>

class ServiceManager;

class AbstractService : public QObject
{
    Q_OBJECT

public:

    explicit AbstractService(QObject *parent = nullptr);
    virtual ~AbstractService();

    // 注册该服务处理的命令列表
    virtual QStringList registeredCommands() const = 0;
    // 服务名称，默认返回类名
    virtual QString serviceName() const;

    // 设置关联的 ServiceManager 实例
    void setServiceManager(ServiceManager* manager);

public slots:

    // 处理接收到的消息
    virtual void onMessageReceived(const QJsonDocument& doc) = 0;

signals:

    // 各服务通过此信号发送消息
    void messageToSend(const QJsonDocument& doc);

protected:

    // 内部调用 messageToSend 信号发送消息
    void sendMessage(const QJsonDocument& doc);
    // 获取关联的 ServiceManager 实例
    ServiceManager* serviceManager() const;
    

private:
    ServiceManager* m_manager;      // 指向关联的 ServiceManager 实例
};

#endif // ABSTRACTSERVICE_H