#include "appcontext.h"

AppContext* AppContext::m_instance = nullptr;

AppContext::AppContext(QObject *parent)
    : QObject(parent),
    m_serviceManager(this),
    m_backlightService(this),
    m_audioService(this),
    m_sysinfoService(this),
    m_wifiService(this),
    m_settingPresenter(this)
{
}

AppContext::~AppContext()
{

}

SettingPresenter *AppContext::settingPresenter()
{
    return &m_settingPresenter;
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

    // presenters 和各个 services 的信号槽连接
    // settingPresenter
    // backlight
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestBacklightChange, &m_backlightService, &BacklightService::onSetBrightness);
    QObject::connect(&m_backlightService, &BacklightService::brightnessSetResult, &m_settingPresenter, &SettingPresenter::handleBacklightChangeResult);
    // audio
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestVolumeChange, &m_audioService, &AudioService::onSetVolume);
    QObject::connect(&m_audioService, &AudioService::volumeSetResult, &m_settingPresenter, &SettingPresenter::handleVolumeChangeResult);

    return 0;
}

AppContext *AppContext::getInstance()
{
    if (!m_instance)
    {
        m_instance = new AppContext();
    }
    return m_instance;
}

















