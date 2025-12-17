#include "servicemanager.h"
#include "core/network/network.h"
#include "abstractservice.h"
#include "utils/log/logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

ServiceManager::ServiceManager(QObject *parent)
    : QObject(parent)
    , m_network(nullptr)
{
    // 构造函数
    // LOG_DEBUG("ServiceManager Constructor called.");
}

ServiceManager::~ServiceManager()
{
    // 析构函数
    // LOG_DEBUG("ServiceManager Destructor called, cleaning up.");
}

bool ServiceManager::initialize(const QString& socketPath)
{
    if (m_network) 
    {
        LOG_WARN("ServiceManager Already initialized. Ignoring re-initialization.");
        return true;
    }

    m_socketPath = socketPath;

    m_network = new Network(this);

    // 连接 Network 的信号到 ServiceManager 的槽
    connect(m_network, &Network::dataReceived, this, &ServiceManager::onDataReceived);
    connect(m_network, &Network::connectionStatusChanged, this, &ServiceManager::onNetworkConnectionStatusChanged);

    LOG_INFO("ServiceManager initialized with socket path: %s", socketPath.toLocal8Bit().constData());
    return true;
}

void ServiceManager::addService(AbstractService* service)
{
    if (!service) 
    {
        LOG_WARN("ServiceManager Tried to add null service pointer.");
        return;
    }

    if (!m_network) 
    {
        LOG_WARN("ServiceManager Not initialized. Cannot add service.");
        return;
    }

    // 1. 将服务添加到服务列表
    m_services.append(service);

    // 2. 获取服务能处理的命令列表
    QStringList commands = service->registeredCommands();
    QString serviceName = service->serviceName();
    LOG_INFO("ServiceManager Adding service '%s' handling commands: %s",
              serviceName.toLocal8Bit().constData(),
              commands.join(", ").toLocal8Bit().constData());

    // 3. 填充路由表
    for (const QString& command : commands) 
    {
        if (m_commandRoutingMap.contains(command)) 
        {
            // 命令已被其他服务处理，发出警告
            LOG_WARN("ServiceManager Command '%s' is already handled by service '%s'. Overriding with service '%s'.",
                     command.toLocal8Bit().constData(),
                     m_commandRoutingMap[command]->serviceName().toLocal8Bit().constData(),
                     serviceName.toLocal8Bit().constData());
        }
        // 建立命令到服务实例的映射
        m_commandRoutingMap[command] = service;
    }

    // 4. 连接服务的 messageToSend 信号到 ServiceManager 的发送槽
    connect(service, &AbstractService::messageToSend,
            this, &ServiceManager::onServiceSendMessage);

    // 5. 设置服务的管理器指针 (如果需要服务访问管理器本身)
    service->setServiceManager(this);

    LOG_INFO("ServiceManager Service '%s' added successfully.",
             serviceName.toLocal8Bit().constData());
}

bool ServiceManager::connectToServer()
{
    if (!m_network) 
    {
        LOG_WARN("ServiceManager Not initialized. Cannot connect to server.");
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
        LOG_WARN("ServiceManager Not initialized. Cannot disconnect.");
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
        LOG_WARN("ServiceManager Received data but Network is not initialized.");
        return;
    }

    // 打印接收到的原始数据
    LOG_DEBUG("ServiceManager Data received from Network, size: %d bytes.", data.size());
    LOG_DEBUG("receive: <<<<< %s", QString(data).toLocal8Bit().constData());

    // 1. 将收到的原始数据 (QByteArray) 解析为 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) 
    {
        LOG_WARN("ServiceManager Failed to parse received data as JSON: %s",
                 parseError.errorString().toLocal8Bit().constData());
        return;
    }

    if (!doc.isObject()) 
    {
        LOG_WARN("ServiceManager Received JSON data is not an object.");
        return;
    }

    QJsonObject obj = doc.object();
    QString command = obj["cmd"].toString();

    if (command.isEmpty()) 
    {
        LOG_WARN("ServiceManager Received JSON without 'cmd' field.");
        return;
    }

    // 2. 查找路由表，看哪个服务能处理这个命令
    if (m_commandRoutingMap.contains(command))
    {
        AbstractService *targetService = m_commandRoutingMap[command];
        LOG_DEBUG("ServiceManager Routing command '%s' to service '%s'.",
                  command.toLocal8Bit().constData(),
                  targetService->serviceName().toLocal8Bit().constData());

        // 3. 将 JSON 消息转发给对应的服务
        targetService->onMessageReceived(doc);
    } 
    else 
    {
        LOG_WARN("ServiceManager No service registered to handle command '%s'.",
                 command.toLocal8Bit().constData());
    }
}

void ServiceManager::onServiceSendMessage(const QJsonDocument& doc)
{
    if (!m_network) 
    {
        LOG_WARN("ServiceManager Not initialized. Cannot send message.");
        return;
    }

    if (!m_network->isConnected()) 
    {
        LOG_WARN("ServiceManager Not connected to server. Cannot send message.");
        return;
    }

    // 将 JSON 文档转换为紧凑格式的 QByteArray
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    if (jsonData.isEmpty()) 
    {
        LOG_WARN("ServiceManager Attempted to send empty JSON message.");
        return;
    }

    LOG_DEBUG("ServiceManager Sending message from service, size: %d bytes.", jsonData.size());
    LOG_DEBUG("Send: >>>>> %s", QString(jsonData).toLocal8Bit().constData());

    // 通过 Network 发送数据
    m_network->sendData(jsonData);
}

void ServiceManager::onNetworkConnectionStatusChanged(bool isConnected)
{
    LOG_DEBUG("ServiceManager Network connection status changed. Now connected: %s",
              isConnected ? "true" : "false");
    // 将 Network 的连接状态变化转发出去
    emit connectionStatusChanged(isConnected);
}
