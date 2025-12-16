#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QJsonDocument>

class Network;
class AbstractService;

class ServiceManager : public QObject
{
    Q_OBJECT

public:

    explicit ServiceManager(QObject *parent = nullptr); 
    virtual ~ServiceManager();

    // Initialize 初始化 ServiceManager 并连接到指定的服务器套接字路径
    bool initialize(const QString& socketPath);
    // 注册一个服务实例
    void addService(AbstractService* service);

    // 连接到服务器
    bool connectToServer();
    // 断开与服务器的连接
    void disconnectFromServer();
    // 检查当前连接状态
    bool isConnected() const;

signals:
    // 连接状态改变时发送的信号
    void connectionStatusChanged(bool isConnected);

private slots:
    // 处理由 Network 接收到的原始数据
    void onDataReceived(const QByteArray& data);

    // 处理由服务发起的发送消息请求
    void onServiceSendMessage(const QJsonDocument& doc);

    // 处理由 Network 发出的连接状态变化
    void onNetworkConnectionStatusChanged(bool isConnected);

private:
    Network* m_network;                                     // 网络通信实例
    QString m_socketPath;                                   // 服务器套接字路径
    QMap<QString, AbstractService*> m_commandRoutingMap;    // 命令到服务的路由表
    QList<AbstractService*> m_services;                     // 已注册的服务列表
};

#endif // SERVICEMANAGER_H