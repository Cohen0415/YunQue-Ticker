#include "sysinfopresenter.h"
#include "utils/log/logger.h"

SysinfoPresenter::SysinfoPresenter(QObject *parent)
    : QObject{parent}
{}

// 接收到 View 的获取 CPU 温度请求
void SysinfoPresenter::onGetCpuTempRequested()
{
    // 向 service 发送获取 CPU 温度请求信号
    emit getCpuTempRequested();
}

// 接收到 View 的获取北京时间请求
void SysinfoPresenter::onGetBjTimeRequested()
{
    // 向 service 发送获取北京时间请求信号
    emit getBjTimeRequested();
}

// 接收到 View 的获取系统版本请求
void SysinfoPresenter::onGetSysVersionRequested()
{
    // 向 service 发送获取系统版本请求信号
    emit getSysVersionRequested();
}

// 接收到 Service 的 CPU 温度获取结果
void SysinfoPresenter::handleCpuTempGetResult(bool success, double value)
{
    // 向 View 发送 CPU 温度获取结果信号
    emit cpuTempGetResult(success, value);
}

// 接收到 Service 的北京时间获取结果
void SysinfoPresenter::handleBjTimeGetResult(bool success, const QString &value)
{
    // 向 View 发送北京时间获取结果信号
    emit bjTimeGetResult(success, value);
}

// 接收到 Service 的系统版本获取结果
void SysinfoPresenter::handleSysVersionGetResult(bool success, const QString &value)
{
    // 向 View 发送系统版本获取结果信号
    emit sysVersionGetResult(success, value);
}
