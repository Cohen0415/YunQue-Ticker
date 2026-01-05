#ifndef SYSINFOPRESENTER_H
#define SYSINFOPRESENTER_H

#include <QObject>

class SysinfoPresenter : public QObject
{
    Q_OBJECT

public:

    explicit SysinfoPresenter(QObject *parent = nullptr);

signals:

    // ====== 向 Service 请求 =====
    void getCpuTempRequested();     // 获取 CPU 温度请求
    void getBjTimeRequested();      // 获取北京时间请求
    void getSysVersionRequested();  // 获取系统版本请求

    // ====== 向 View/Model 发结果 =====
    void cpuTempGetResult(bool success, double value);        // CPU 温度获取结果
    void bjTimeGetResult(bool success, const QString &value); // 北京时间获取结果
    void sysVersionGetResult(bool success, const QString &value); // 系统版本获取结果

public slots:

    // ====== 接收 View 的请求 =====
    void onGetCpuTempRequested();     // 获取 CPU 温度请求
    void onGetBjTimeRequested();      // 获取北京时间请求
    void onGetSysVersionRequested();  // 获取系统版本请求

    // ====== 接收 Service 的返回 =====
    void handleCpuTempGetResult(bool success, double value);        // CPU 温度获取结果
    void handleBjTimeGetResult(bool success, const QString &value); // 北京时间获取结果
    void handleSysVersionGetResult(bool success, const QString &value); // 系统版本获取结果

};

#endif // SYSINFOPRESENTER_H
