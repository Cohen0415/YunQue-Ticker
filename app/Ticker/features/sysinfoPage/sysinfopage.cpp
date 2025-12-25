#include "sysinfopage.h"
#include "ui_sysinfopage.h"
#include "utils/log/logger.h"
#include "features/pagemsgmanager.h"
#include "utils/log/logger.h"

#include <QFile>

SysinfoPage::SysinfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SysinfoPage)
    , m_sysinfoRefreshTimer(new QTimer(this))
    , m_bjTimerRefreshTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 连接定时器信号槽
    connect(m_sysinfoRefreshTimer, &QTimer::timeout, this, &SysinfoPage::onSysInfoRefreshTimeout);
    connect(m_bjTimerRefreshTimer, &QTimer::timeout, this, &SysinfoPage::onBJTimeRefreshTimeout);
    m_bjTimerRefreshTimer->start(BJTIME_REFRESH_INTERVAL_MS);
}

SysinfoPage::~SysinfoPage()
{
    delete ui;
}

// 页面初始化
void SysinfoPage::init()
{
    uiInit();

    // 请求获取 CPU 温度
    emit getCpuTempRequested();
    // 请求获取北京时间
    emit getBjTimeRequested();
    // 更新系统运行时间
    updateSysRunTime();
}

// 页面进入回调
void SysinfoPage::onPageEnter()
{
    LOG_DEBUG("SysinfoPage entered.");
    // 启动系统信息刷新定时器
    if (!m_sysinfoRefreshTimer->isActive())
        m_sysinfoRefreshTimer->start(SYSINFO_REFRESH_INTERVAL_MS);
}

// 页面离开回调
void SysinfoPage::onPageLeave()
{
    LOG_DEBUG("SysinfoPage left.");
    // 停止系统信息刷新定时器
    if (m_sysinfoRefreshTimer->isActive())
        m_sysinfoRefreshTimer->stop();
}

// 接收到 presenter 的 CPU 温度获取结果
void SysinfoPage::onCpuTempGetResult(bool success, double value)
{
    // 例如：45.326 摄氏度
    if (success)
    {
        ui->cpuTempLabel->setText(
            m_infoCpuTempPrefixStr + QString::number(value, 'f', 2) + "C"
            );
    }
    else
    {
        ui->cpuTempLabel->setText(m_infoCpuTempPrefixStr + "获取失败");
    }
}

// 接收到 presenter 的北京时间获取结果
void SysinfoPage::onBjTimeGetResult(bool success, const QString &value)
{
    // 例如："2025-12-05T09:26:25+08:00"
    emit PageMsgManager::getInstance()->bjTimeUpdated(value);
}

// 系统信息刷新定时器超时槽函数
void SysinfoPage::onSysInfoRefreshTimeout()
{
    // 请求获取 CPU 温度
    emit getCpuTempRequested();

    // 更新系统运行时间
    updateSysRunTime();
}

// 北京时间刷新定时器超时槽函数
void SysinfoPage::onBJTimeRefreshTimeout()
{
    // 请求获取北京时间
    emit getBjTimeRequested();
}

// UI 初始化
void SysinfoPage::uiInit()
{
    // 初始化各个提示语为空
    ui->sysVerLabel->setText(m_infoSysVerPrefixStr);
    ui->appVerLabel->setText(m_infoAppVerPrefixStr);
    ui->cpuTempLabel->setText(m_infoCpuTempPrefixStr);
    ui->sysRunTimeLabel->setText(m_infoSysRunTimePrefixStr);
}

// 更新系统运行时间
void SysinfoPage::updateSysRunTime()
{
    // 读取 /proc/uptime
    QString uptimeFilePath = "/proc/uptime";
    QFile file(uptimeFilePath);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&file);
        QString line = in.readLine();
        QStringList parts = line.split(" ");
        if (!parts.isEmpty())
        {
            bool ok = false;
            double uptimeSeconds = parts[0].toDouble(&ok);
            if (ok)
            {
                int totalSeconds = static_cast<int>(uptimeSeconds);
                int days = totalSeconds / 86400;
                int hours = (totalSeconds % 86400) / 3600;
                int minutes = (totalSeconds % 3600) / 60;
                QString runTimeStr;
                if (days > 0)
                    runTimeStr = QString("%1 d %2 h %3 m").arg(days).arg(hours).arg(minutes);
                else if (hours > 0)
                    runTimeStr = QString("%1 h %2 m").arg(hours).arg(minutes);
                else
                    runTimeStr = QString("%1 m").arg(minutes);
                ui->sysRunTimeLabel->setText(m_infoSysRunTimePrefixStr + runTimeStr);
            }
        }
        file.close();
    }
}
