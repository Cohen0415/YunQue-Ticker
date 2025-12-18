#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>

#include "utils/log/logger.h"
#include "services/servicemanager.h"
#include "services/backlight/backlightservice.h"
#include "services/audio/audioservice.h"
#include "services/sysinfo/sysinfoservice.h"
#include "services/wifi/wifiservice.h"

#define UDS_PATH    "/tmp/dev.sock"

class AppContext : public QObject
{
    Q_OBJECT

public:

    explicit AppContext(QObject *parent = nullptr);
    ~AppContext();

    int init(void);

signals:

private:

    ServiceManager m_serviceManager;
    BacklightService m_backlightService;
    AudioService m_audioService;
    SysinfoService m_sysinfoService;
    WifiService m_wifiService;

};

#endif // APPCONTEXT_H
