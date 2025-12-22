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

int AppContext::Init()
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
    // backlight set
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestBacklightSetChange, &m_backlightService, &BacklightService::onSetBrightness);
    QObject::connect(&m_backlightService, &BacklightService::brightnessSetResult, &m_settingPresenter, &SettingPresenter::handleBacklightSetChangeResult);
    // backlight get
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestBacklightGetChange, &m_backlightService, &BacklightService::onGetBrightness);
    QObject::connect(&m_backlightService, &BacklightService::brightnessGetResult, &m_settingPresenter, &SettingPresenter::handleBacklightGetChangeResult);
    // audio set
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestVolumeSetChange, &m_audioService, &AudioService::onSetVolume);
    QObject::connect(&m_audioService, &AudioService::volumeSetResult, &m_settingPresenter, &SettingPresenter::handleVolumeSetChangeResult);
    // audio get
    QObject::connect(&m_settingPresenter, &SettingPresenter::requestVolumeGetChange, &m_audioService, &AudioService::onGetVolume);
    LOG_DEBUG("000000");
    QObject::connect(&m_audioService, &AudioService::volumeGetResult, &m_settingPresenter, &SettingPresenter::handleVolumeGetChangeResult);

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

















