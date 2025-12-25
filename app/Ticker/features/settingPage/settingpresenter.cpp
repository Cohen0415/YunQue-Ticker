#include "settingpresenter.h"
#include "utils/log/logger.h"

SettingPresenter::SettingPresenter(QObject* parent)
    : QObject(parent)
{

}

// 接收到 View 发来的背光设置请求
void SettingPresenter::onBacklightSetChangeRequested(int value)
{
    // 向 Service 发出请求信号
    emit requestBacklightSetChange(value);
}

// 接收到 View 发来的背光获取请求
void SettingPresenter::onBacklightGetChangeRequested()
{
    // 向 Service 发出请求信号
    emit requestBacklightGetChange();
}

// 接收到 View 发来的音量设置请求
void SettingPresenter::onVolumeSetChangeRequested(int value)
{
    // 向 Service 发出请求信号
    emit requestVolumeSetChange(value);
}

// 接收到 View 发来的音量获取请求
void SettingPresenter::onVolumeGetChangeRequested()
{
    // 向 Service 发出请求信号
    emit requestVolumeGetChange();
}

// 处理来自 Service 的背光设置结果
void SettingPresenter::handleBacklightSetChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit backlightSetChangeResult(success, value);
}

// 处理来自 Service 的背光获取结果
void SettingPresenter::handleBacklightGetChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit backlightGetChangeResult(success, value);
}

// 处理来自 Service 的音量设置结果
void SettingPresenter::handleVolumeSetChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit volumeSetChangeResult(success, value);
}

// 处理来自 Service 的音量获取结果
void SettingPresenter::handleVolumeGetChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit volumeGetChangeResult(success, value);
}
