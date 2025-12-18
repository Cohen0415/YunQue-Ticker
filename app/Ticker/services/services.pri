include($$PWD/backlight/backlight.pri)
include($$PWD/audio/audio.pri)
include($$PWD/sysinfo/sysinfo.pri)
include($$PWD/wifi/wifi.pri)

HEADERS += \
    $$PWD/abstractservice.h \
    $$PWD/servicemanager.h

SOURCES += \
    $$PWD/abstractservice.cpp \
    $$PWD/servicemanager.cpp
