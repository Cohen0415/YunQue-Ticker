#include "sysinfopage.h"
#include "ui_sysinfopage.h"
#include "utils/log/logger.h"
#include "features/pagemsgmanager.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"
#include <QFile>
#include <QRegularExpression>

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
    // 不再启动北京时间刷新定时器，由框架页自己使用 Qt 接口获取系统时间
    // m_bjTimerRefreshTimer->start(BJTIME_REFRESH_INTERVAL_MS);
}

SysinfoPage::~SysinfoPage()
{
    delete ui;
}

// 页面初始化
void SysinfoPage::init()
{
    uiInit();

    // 更新 app 版本
    updateAppVersion();

    // 更新系统版本
    emit getSysVersionRequested();
}

// 页面进入回调
void SysinfoPage::onPageEnter()
{
    LOG_DEBUG("SysinfoPage entered.");
    // 请求获取 CPU 温度
    emit getCpuTempRequested();
    // 请求获取北京时间
    // emit getBjTimeRequested();
    // 更新系统运行时间
    updateSysRunTime();
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

// 接收到 presenter 的系统版本获取结果
void SysinfoPage::onSysVersionGetResult(bool success, const QString &value)
{
    // 例如："v1.0.0"
    if (success)
    {
        ui->sysVerLabel->setText(m_infoSysVerPrefixStr + value);
    }
    else
    {
        ui->sysVerLabel->setText(m_infoSysVerPrefixStr + "获取失败");
    }
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

    // 加载样式表
    QString style = QssLoader::load(":/res/qss/pageQss/sysinfoPage.qss");
    if (!style.isEmpty())
    {
        this->setStyleSheet(style);
    }

    // 默认不显示更新按钮
    ui->appUpdateBtn->setVisible(false);
    ui->sysUpdateBtn->setVisible(false);
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
                    runTimeStr = QString("%1 天 %2 时 %3 min").arg(days).arg(hours).arg(minutes);
                else if (hours > 0)
                    runTimeStr = QString("%1 时 %2 min").arg(hours).arg(minutes);
                else
                    runTimeStr = QString("%1 min").arg(minutes);
                ui->sysRunTimeLabel->setText(m_infoSysRunTimePrefixStr + runTimeStr);
            }
        }
        file.close();
    }
}

// 更新 app 版本号
void SysinfoPage::updateAppVersion()
{
    QString normalizedVersion = normalizeVersion(APP_GIT_VERSION);
    ui->appVerLabel->setText(m_infoAppVerPrefixStr + normalizedVersion);
}

// 版本号规范显示
QString SysinfoPage::normalizeVersion(const QString &gitDesc)
{
    // 从 v0.5.0-9-gxxxx 变成 v0.5.9
    static QRegularExpression re(
        R"(^(v\d+\.\d+)\.\d+-(\d+)-g[0-9a-f]+)"
    );

    QRegularExpressionMatch m = re.match(gitDesc);
    if (!m.hasMatch())
        return gitDesc;   

    QString base = m.captured(1);   
    QString dist = m.captured(2);   

    return base + "." + dist;
}