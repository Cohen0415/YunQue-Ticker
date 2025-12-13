QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

# 核心层，主要负责数据的发送和接收
include($$PWD/core/core.pri)

# 接口层，向上提供数据获取接口（比如背光、音频、股票数据等）
include($$PWD/services/services.pri)

# 数据模型层
include($$PWD/models/models.pri)

# presenter 和 view 层，其中 presenter 是 services 与 view 的桥梁
# view 层获取用户事件，向 presenter 发送对应的信号 --->
# presenter 层接收来自 view 层的信号，分析 view 层需要哪个数据模型的数据 --->
# presenter 去 services 层调用对应的模块接口获取数据，并构造数据模型 --->
# 将构造好的数据模型返回给 view 层 --->
# view 层根据数据刷新 UI
include($$PWD/features/features.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
