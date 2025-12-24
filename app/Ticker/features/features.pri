include($$PWD/homePage/homePage.pri)
include($$PWD/settingPage/settingPage.pri)
include($$PWD/sysinfoPage/sysinfoPage.pri)
include($$PWD/wifiPage/wifiPage.pri)

HEADERS += \
    $$PWD/pagelifecycleaware.h \
    $$PWD/pagemsgmanager.h

SOURCES += \
    $$PWD/pagemsgmanager.cpp