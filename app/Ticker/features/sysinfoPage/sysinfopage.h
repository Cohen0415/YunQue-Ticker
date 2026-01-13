#ifndef SYSINFOPAGE_H
#define SYSINFOPAGE_H

#include <QWidget>
#include <QTimer>
#include "features/pagelifecycleaware.h"

#define SYSINFO_REFRESH_INTERVAL_MS     3000    // 系统信息刷新定时器间隔
#define BJTIME_REFRESH_INTERVAL_MS      10000   // 北京时间刷新间隔

namespace Ui {
class SysinfoPage;
}

class SysinfoPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit SysinfoPage(QWidget *parent = nullptr);
    ~SysinfoPage();

    void init();                    // 初始化

    // PageLifecycleAware 接口实现
    void onPageEnter() override;    // 页面进入时调用
    void onPageLeave() override;    // 页面离开时调用

signals:

    // 向 presenter 发送的信号
    void getCpuTempRequested();     // 获取 CPU 温度请求
    void getBjTimeRequested();      // 获取北京时间请求
    void getSysVersionRequested();  // 获取系统版本请求

public slots:

    // 接收 presenter 发送的结果
    void onCpuTempGetResult(bool success, double value);            // CPU 温度获取结果
    void onBjTimeGetResult(bool success, const QString &value);     // 北京时间获取结果
    void onSysVersionGetResult(bool success, const QString &value); // 系统版本获取结果

    // 系统信息刷新定时器超时槽函数
    void onSysInfoRefreshTimeout(); // 系统信息刷新超时槽函数
    void onBJTimeRefreshTimeout();  // 北京时间刷新超时槽函数

private:

    void uiInit();                  // UI 初始化
    void updateSysRunTime();        // 获取系统运行时间
    void updateAppVersion();        // 更新 app 版本号
    QString normalizeVersion(const QString &gitDesc); // 版本号规范显示

    QString m_infoSysVerPrefixStr = QStringLiteral("系统固件版本：");      // 系统固件版本前缀字符串
    QString m_infoAppVerPrefixStr = QStringLiteral("APP 版本：");         // 应用程序版本前缀字符串
    QString m_infoCpuTempPrefixStr = QStringLiteral("CPU 温度：");        // CPU 温度前缀字符串
    QString m_infoSysRunTimePrefixStr = QStringLiteral("系统运行时间：");   // 系统运行时间前缀字符串

    QTimer *m_sysinfoRefreshTimer;          // 系统信息刷新定时器
    QTimer *m_bjTimerRefreshTimer;          // 北京时间刷新定时器

    Ui::SysinfoPage *ui;
};

#endif // SYSINFOPAGE_H
