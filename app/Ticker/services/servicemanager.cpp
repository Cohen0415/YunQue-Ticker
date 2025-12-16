#include "servicemanager.h"
#include "core/network/network.h"
#include "abstractservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
    , m_network(nullptr)
{
    // 构造函数
    qDebug() << "[ServiceManager] Created.";
}

ServiceManager::~ServiceManager()
{
    // 析构函数
    qDebug() << "[ServiceManager] Destroyed.";
}

bool ServiceManager::initialize(const QString& socketPath)
{
    if (m_network) 
    {
        qWarning() << "[ServiceManager] Already initialized.";
        return true;
    }

    m_socketPath = socketPath;

    m_network = new Network(this);

    // 连接 Network 的信号到 ServiceManager 的槽
    connect(m_network, &Network::dataReceived, this, &ServiceManager::onDataReceived);
    connect(m_network, &Network::connectionStatusChanged, this, &ServiceManager::onNetworkConnectionStatusChanged);

    qDebug() << "[ServiceManager] Initialized with socket path:" << socketPath;
    return true;
}

void ServiceManager::addService(AbstractService* service)
{
    if (!service) 
    {
        qWarning() << "[ServiceManager] Attempted to add a null service.";
        return;
    }

    if (!m_network) 
    {
        qWarning() << "[ServiceManager] Cannot add service before initialization.";
        return;
    }

    // 1. 将服务添加到服务列表
    m_services.append(service);

    // 2. 获取服务能处理的命令列表
    QStringList commands = service->registeredCommands();
    QString serviceName = service->serviceName();
    qDebug() << "[ServiceManager] Adding service" << serviceName
             << "which handles commands:" << commands;

    // 3. 填充路由表
    for (const QString& command : commands) 
    {
        if (m_commandRoutingMap.contains(command)) 
        {
            // 命令已被其他服务处理，发出警告
            qWarning() << "[ServiceManager] Command" << command
                       << "is already handled by service"
                       << m_commandRoutingMap[command]->serviceName()
                       << ". New service" << serviceName
                       << "will override it.";
        }
        // 建立命令到服务实例的映射
        m_commandRoutingMap[command] = service;
    }

    // 4. 连接服务的 messageToSend 信号到 ServiceManager 的发送槽
    connect(service, &AbstractService::messageToSend,
            this, &ServiceManager::onServiceSendMessage);

    // 5. 设置服务的管理器指针 (如果需要服务访问管理器本身)
    service->setServiceManager(this);

    qDebug() << "[ServiceManager] Service" << serviceName << "added successfully.";
}

bool ServiceManager::connectToServer()
{
    if (!m_network) 
    {
        qWarning() << "[ServiceManager] Not initialized. Cannot connect.";
        return false;
    }

    return m_network->connectToServer(m_socketPath);
}

void ServiceManager::disconnectFromServer()
{
    if (m_network) 
    {
        m_network->disconnectFromServer();
    } 
    else 
    {
        qDebug() << "[ServiceManager] Not initialized. Nothing to disconnect.";
    }
}

bool ServiceManager::isConnected() const
{
    if (m_network) 
    {
        return m_network->isConnected();
    }

    return false;
}

void ServiceManager::onDataReceived(const QByteArray& data)
{
    if (!m_network) 
    {
        qWarning() << "[ServiceManager] Received data but Network is not initialized.";
        return;
    }

    // 打印接收到的原始数据
    qDebug() << "[ServiceManager] Data received from Network, size:" << data.size() << "bytes.";
    qDebug() << "Raw data content:" << data;

    // 1. 将收到的原始数据 (QByteArray) 解析为 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) 
    {
        qWarning() << "[ServiceManager] Failed to parse incoming JSON data:" << parseError.errorString();
        return;
    }

    if (!doc.isObject()) 
    {
        qWarning() << "[ServiceManager] Received JSON data is not an object.";
        return;
    }

    QJsonObject obj = doc.object();
    QString command = obj["cmd"].toString();

    if (command.isEmpty()) 
    {
        qWarning() << "[ServiceManager] Received JSON object lacks a 'cmd' field.";
        return;
    }

    // 2. 查找路由表，看哪个服务能处理这个命令
    if (m_commandRoutingMap.contains(command))
    {
        AbstractService *targetService = m_commandRoutingMap[command];
        qDebug() << "[ServiceManager] Routing command" << command << "to service" << targetService->serviceName();

        // 3. 将 JSON 消息转发给对应的服务
        targetService->onMessageReceived(doc);
    } 
    else 
    {
        qWarning() << "[ServiceManager] No service registered to handle command:" << command;
    }
}

void ServiceManager::onServiceSendMessage(const QJsonDocument& doc)
{
    if (!m_network) 
    {
        qWarning() << "[ServiceManager] Tried to send message but Network is not initialized.";
        return;
    }

    if (!m_network->isConnected()) 
    {
        qWarning() << "[ServiceManager] Cannot send message, Network is not connected.";
        return;
    }

    // 将 JSON 文档转换为紧凑格式的 QByteArray
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    if (jsonData.isEmpty()) 
    {
        qWarning() << "[ServiceManager] Attempted to send empty or invalid JSON from service.";
        return;
    }

    qDebug() << "[ServiceManager] Sending JSON message from service, size:" << jsonData.size() << "bytes.";
    qDebug() << "Message content:" << doc.toJson(QJsonDocument::Indented); // 调试用，打印格式化JSON

    // 通过 Network 发送数据
    m_network->sendData(jsonData);
}

void ServiceManager::onNetworkConnectionStatusChanged(bool isConnected)
{
    qDebug() << "[ServiceManager] Network connection status changed to:" << (isConnected ? "Connected" : "Disconnected");
    // 将 Network 的连接状态变化转发出去
    emit connectionStatusChanged(isConnected);
}
