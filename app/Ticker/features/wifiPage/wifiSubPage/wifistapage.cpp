#include "wifistapage.h"
#include "ui_wifistapage.h"
#include "features/pagemsgmanager.h"
#include "utils/log/logger.h"
#include <QFile>

WifiStaPage::WifiStaPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiStaPageWidget)
    , m_statusTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 定时轮询 wifi 连接情况
    connect(m_statusTimer, &QTimer::timeout, this, &WifiStaPage::onStatusTimerTimeout);
}

WifiStaPage::~WifiStaPage()
{
    delete ui;
}

// 页面初始化，给父页面调用
void WifiStaPage::init()
{
    // UI 初始化
    uiInit();
}

// 页面进入回调
void WifiStaPage::onPageEnter()
{
    LOG_DEBUG("WifiStaPage entered.");

    // 启动状态轮询定时器
    if (!m_statusTimer->isActive())
        m_statusTimer->start(REFRESH_WIFI_STA_MS);
}

// 页面离开回调
void WifiStaPage::onPageLeave()
{
    LOG_DEBUG("WifiStaPage left.");

    // 关闭状态轮询定时器
    if (m_statusTimer->isActive())
        m_statusTimer->stop();
}

// 接收父页面发送的获取 wifi 状态结果
void WifiStaPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    if (success && connected)
    {
        // 向 PageMsgManager 发送已连接信号
        emit PageMsgManager::getInstance()->wifiStatusChanged(true);

        // 更新 wifi 名称显示
        ui->ssidLabel->setText(ssid);

        // 更新按钮状态
        ui->disconnButton->setText("断 开 连 接");
        ui->disconnButton->setEnabled(true);

        // 获取当前信号等级
        WifiRssiLevel level = getRssiLevel(rssi.toInt());
        LOG_DEBUG("m_currentRssiLevel = %d", m_currentRssiLevel);
        LOG_DEBUG("level = %d", level);
        if (level != m_currentRssiLevel)
        {
            // 更新信号图标
            m_currentRssiLevel = level;
            QString iconPath = QString(":/res/icon/pageIcon/wifiPage/wifi-%1.png").arg(static_cast<int>(m_currentRssiLevel));
            ui->rssiIconLabel->setPixmap(QPixmap(iconPath));
        }

        // 如果是第一次启动，先停止轮询定时器
        if (m_startFlag == 1)
        {
            if (m_statusTimer->isActive())
                m_statusTimer->stop();
            m_startFlag == 0;
        }
    }
    else if (success && !connected)
    {
        // 停止状态轮询定时器
        if (m_statusTimer->isActive())
            m_statusTimer->stop();

        // 向 PageMsgManager 发送断开连接信号
        emit PageMsgManager::getInstance()->wifiStatusChanged(false);

        // wifi 断开连接，切换到连接页面
        emit switchToConnPageRequested();

        // 复位按钮
        ui->disconnButton->setText("断 开 连 接");
        ui->disconnButton->setEnabled(true);
    }
}

// 轮询定时器槽函数
void WifiStaPage::onStatusTimerTimeout()
{
    emit getWifiStatusRequested();
}

// UI 初始化
void WifiStaPage::uiInit()
{
    // 加载样式表
    QString style = loadQssStyle(":/res/qss/pageQss/wifiStaPage.qss");
    if (!style.isEmpty())
    {
        this->setStyleSheet(style);
    }

    // 默认零格信号
    ui->rssiIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/wifiPage/wifi-0.png"));

    // wifi 名称默认空
    ui->ssidLabel->setText("");

    // 复位按钮提示语
    ui->disconnButton->setText("断 开 连 接");
    ui->disconnButton->setFocusPolicy(Qt::NoFocus);
}

// 获取当前信号等级
WifiRssiLevel WifiStaPage::getRssiLevel(const int &rssi)
{
    if (rssi >= -50)
    {
        return WIFI_RSSI_LEVEL_4;
    }
    else if (rssi >= -60)
    {
        return WIFI_RSSI_LEVEL_3;
    }
    else if (rssi >= -70)
    {
        return WIFI_RSSI_LEVEL_2;
    }
    else if (rssi >= -80)
    {
        return WIFI_RSSI_LEVEL_1;
    }
    else
    {
        return WIFI_RSSI_LEVEL_0;
    }
}

// 加载样式表
QString WifiStaPage::loadQssStyle(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return QString(); // 打开失败返回空字符串

    QTextStream in(&file);
    in.setCodec("UTF-8"); // 保证中文正常显示

    QString style = in.readAll();
    file.close();
    return style;
}

// 断开连接按钮槽函数
void WifiStaPage::on_disconnButton_clicked()
{
    // 向父页面发送断开连接请求信号
    emit disconnectWifiRequested();

    // 禁用按钮
    ui->disconnButton->setText("断 开 连 接 中...");
    ui->disconnButton->setEnabled(false);
}
