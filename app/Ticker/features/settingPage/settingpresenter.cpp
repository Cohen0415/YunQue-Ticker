#include "settingpresenter.h"
#include "utils/log/logger.h"

SettingPresenter::SettingPresenter(QObject* parent)
    : QObject(parent)
{

}

// 接收到 View 发来的背光设置请求
void SettingPresenter::onBacklightChangeRequested(int value)
{
    // 向 Service 发出请求信号
    emit requestBacklightChange(value);
}

// 接收到 View 发来的音量设置请求
void SettingPresenter::onVolumeChangeRequested(int value)
{
    // 向 Service 发出请求信号
    emit requestVolumeChange(value);
}

// 处理来自 Service 的背光设置结果
void SettingPresenter::handleBacklightChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit backlightChangeResult(success, value);
}

// 处理来自 Service 的音量设置结果
void SettingPresenter::handleVolumeChangeResult(bool success, int value)
{
    // 将结果通过信号发回给 View
    emit volumeChangeResult(success, value);
}


