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
#include "features/sysinfoPage/sysinfopresenter.h"

#define UDS_PATH    "/tmp/dev.sock"

class AppContext : public QObject
{
    Q_OBJECT

public:

    explicit AppContext(QObject *parent = nullptr);
    ~AppContext();

    int init(void);

    static AppContext *getInstance();       // 获取 AppContext 实例

    // 获取 Presenters
    SettingPresenter *settingPresenter();   // 获取 SettingPresenter 对象
    WifiPresenter *wifiPresenter();         // 获取 WifiPresenter 对象
    SysinfoPresenter *sysinfoPresenter();   // 获取 SysinfoPresenter 对象

signals:

public slots:

private:

    static AppContext *m_instance;

    ServiceManager m_serviceManager;
    BacklightService m_backlightService;
    AudioService m_audioService;
    SysinfoService m_sysinfoService;
    WifiService m_wifiService;

    SettingPresenter m_settingPresenter;
    WifiPresenter m_wifiPresenter;
    SysinfoPresenter m_sysinfoPresenter;
};

#endif // APPCONTEXT_H
