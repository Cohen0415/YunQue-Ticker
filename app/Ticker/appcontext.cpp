#include "appcontext.h"
#include <QThread>

AppContext* AppContext::m_instance = nullptr;

AppContext::AppContext(QObject *parent)
    : QObject(parent),
    m_serviceManager(this),
    m_backlightService(this),
    m_audioService(this),
    m_sysinfoService(this),
    m_wifiService(this),
    m_settingPresenter(this),
    m_wifiPresenter(this),
    m_sysinfoPresenter(this)
{
}

AppContext::~AppContext()
{

}

SettingPresenter *AppContext::settingPresenter()
{
    return &m_settingPresenter;
}

WifiPresenter *AppContext::wifiPresenter()
{
    return &m_wifiPresenter;
}

SysinfoPresenter *AppContext::sysinfoPresenter()
{
    return &m_sysinfoPresenter;
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
    LOG_INFO("ServiceManager initialized.");

    // 循环连接服务器，直到连接成功
    while (!m_serviceManager.isConnected())
    {
        LOG_DEBUG("Attempting to connect to server...");
        if (m_serviceManager.connectToServer())
        {
            LOG_INFO("Connected to server successfully.");
            break;
        }
        else
        {
            LOG_WARN("Failed to connect to server. Retrying in 2 seconds...");
            QThread::sleep(2);
        }
    }

    // 添加各个服务到 ServiceManager
    m_serviceManager.addService(&m_backlightService);
    m_serviceManager.addService(&m_audioService);
    m_serviceManager.addService(&m_sysinfoService);
    m_serviceManager.addService(&m_wifiService);
    LOG_INFO("Services added to ServiceManager.");

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
    QObject::connect(&m_audioService, &AudioService::volumeGetResult, &m_settingPresenter, &SettingPresenter::handleVolumeGetChangeResult);

    // wifiPresenter
    // wifi connect
    QObject::connect(&m_wifiPresenter, &WifiPresenter::requestConnectWifi, &m_wifiService, &WifiService::onConnectToNetwork);
    QObject::connect(&m_wifiService, &WifiService::connectResult, &m_wifiPresenter, &WifiPresenter::handleConnectWifiResult);
    // wifi distconnect
    QObject::connect(&m_wifiPresenter, &WifiPresenter::requestDisconnectWifi, &m_wifiService, &WifiService::onDisconnectFromNetwork);
    QObject::connect(&m_wifiService, &WifiService::disconnectResult, &m_wifiPresenter, &WifiPresenter::handleDisconnectWifiResult);
    // wifi get status
    QObject::connect(&m_wifiPresenter, &WifiPresenter::requestGetWifiStatus, &m_wifiService, &WifiService::onGetWifiStatus);
    QObject::connect(&m_wifiService, &WifiService::wifiStatusResult, &m_wifiPresenter, &WifiPresenter::handleGetWifiStatusResult);

    // sysinfoPresenter
    // cpu temp get
    QObject::connect(&m_sysinfoPresenter, &SysinfoPresenter::getCpuTempRequested, &m_sysinfoService, &SysinfoService::onGetCpuTemperature);
    QObject::connect(&m_sysinfoService, &SysinfoService::cpuTemperatureResult, &m_sysinfoPresenter, &SysinfoPresenter::handleCpuTempGetResult);
    // bj time get
    QObject::connect(&m_sysinfoPresenter, &SysinfoPresenter::getBjTimeRequested, &m_sysinfoService, &SysinfoService::onGetBeijingTime);
    QObject::connect(&m_sysinfoService, &SysinfoService::beijingTimeResult, &m_sysinfoPresenter, &SysinfoPresenter::handleBjTimeGetResult);
    // sys version get
    QObject::connect(&m_sysinfoPresenter, &SysinfoPresenter::getSysVersionRequested, &m_sysinfoService, &SysinfoService::onGetSystemVersion);
    QObject::connect(&m_sysinfoService, &SysinfoService::systemVersionResult, &m_sysinfoPresenter, &SysinfoPresenter::handleSysVersionGetResult);

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
