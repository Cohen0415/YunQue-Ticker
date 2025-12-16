#include "abstractservice.h"
#include "servicemanager.h" // 现在需要完整定义
#include <QMetaObject>      // 用于获取类名
#include <QDebug>

AbstractService::AbstractService(QObject *parent)
    : QObject(parent), m_manager(nullptr)
{
    // 构造函数实现
}

AbstractService::~AbstractService()
{
    // 析构函数实现
}

QStringList AbstractService::registeredCommands() const
{
    // 默认实现返回空列表，子类应重写此方法
    return QStringList();
}

QString AbstractService::serviceName() const
{
    // 默认实现返回类名
    return QString(metaObject()->className());
}

void AbstractService::onMessageReceived(const QJsonDocument& /*doc*/)
{
    // 默认实现为空，子类应重写此方法
}

void AbstractService::sendMessage(const QJsonDocument& doc)
{
    emit messageToSend(doc);
}

ServiceManager* AbstractService::serviceManager() const
{
    return m_manager;
}

void AbstractService::setServiceManager(ServiceManager* manager)
{
    if (m_manager != manager) 
    {
        m_manager = manager;
    }
}