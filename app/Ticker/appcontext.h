#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>

#include "utils/log/logger.h"

// services
#include "services/servicemanager.h"
#include "services/backlight/backlightservice.h"
#include "services/audio/audioservice.h"
#include "services/sysinfo/sysinfoservice.h"
#include "services/wifi/wifiservice.h"

// presenters
#include "features/settingPage/settingpresenter.h"
#include "features/wifiPage/wifipresenter.h"

#define UDS_PATH    "/tmp/dev.sock"

class AppContext : public QObject
{
    Q_OBJECT

public:

    explicit AppContext(QObject *parent = nullptr);
    ~AppContext();

    int Init(void);

    // 获取 AppContext 实例
    static AppContext *getInstance();
    // 获取 Presenters
    SettingPresenter *settingPresenter();
    WifiPresenter *wifiPresenter();

signals:

private:

    static AppContext *m_instance;

    ServiceManager m_serviceManager;
    BacklightService m_backlightService;
    AudioService m_audioService;
    SysinfoService m_sysinfoService;
    WifiService m_wifiService;

    SettingPresenter m_settingPresenter;
    WifiPresenter m_wifiPresenter;
};

#endif // APPCONTEXT_H
