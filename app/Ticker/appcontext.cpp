#include "appcontext.h"

AppContext::AppContext(QObject *parent)
    : QObject(parent),
    m_serviceManager(this),
    m_backlightService(this),
    m_audioService(this),
    m_sysinfoService(this),
    m_wifiService(this)
{
}

AppContext::~AppContext()
{

}

int AppContext::init()
{
    // 初始化 ServiceManager
    int ret = m_serviceManager.initialize(UDS_PATH);
    if (ret == false)
    {
        LOG_ERROR("Failed to initialize ServiceManager");
        return -1;
    }

    // 连接到服务器
    ret = m_serviceManager.connectToServer();
    if (ret == false)
    {
        LOG_ERROR("Failed to connect Server");
        return -1;
    }

    // 添加各个服务到 ServiceManager
    m_serviceManager.addService(&m_backlightService);
    m_serviceManager.addService(&m_audioService);
    m_serviceManager.addService(&m_sysinfoService);
    m_serviceManager.addService(&m_wifiService);

    // 连接信号槽
    // todo
    // 建立 presenter's request-singals ---> service's request-solts
    // 建立 service's result-singals ---> presenter's result-slots

    return 0;
}
