include($$PWD/backlight/backlight.pri)
include($$PWD/audio/audio.pri)

HEADERS += \
    $$PWD/abstractservice.h \
    $$PWD/servicemanager.h

SOURCES += \
    $$PWD/abstractservice.cpp \
    $$PWD/servicemanager.cpp