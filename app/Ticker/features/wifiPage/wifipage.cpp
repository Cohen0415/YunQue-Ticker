#include "wifipage.h"
#include "ui_wifipage.h"

WifiPage::WifiPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiPage)
{
    ui->setupUi(this);
}

WifiPage::~WifiPage()
{
    delete ui;
}

void WifiPage::Init()
{
    SubPageInit();
}

// 接收 presenter 发送的 Wi-Fi 连接结果
void WifiPage::onConnectWifiResult(bool success)
{
    // not todo
}

// 接收 presenter 发送的 Wi-Fi 断开连接结果
void WifiPage::onDisconnectWifiResult(bool success)
{
    // not todo
}

// 接收 presenter 发送的获取 Wi-Fi 状态结果
void WifiPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    // 将结果发送给子页面
    emit getWifiStatusResultToSubPage(success, connected, ssid, ip, rssi);
}

// 接收子页面发送的获取状态请求
void WifiPage::onGetStatusRequestFromSubPage()
{
    // 向 presenter 发送获取 Wi-Fi 状态请求信号
    emit getWifiStatusRequested();
}

// 接收子页面发送的断开连接请求
void WifiPage::onDisconnectRequestFromSubPage()
{
    // 向 presenter 发送断开 Wi-Fi 请求信号
    emit disconnectWifiRequested();
}

// 接收 connPage 发送的连接请求
void WifiPage::onConnectRequestFromSubPage(const QString &ssid, const QString &password)
{
    // 向 presenter 发送连接 Wi-Fi 请求信号
    emit connectWifiRequested(ssid, password);
}

// 接收 connPage 的切换到 staPage 的请求
void WifiPage::onSwitchToStaPageRequestFromConnSubPage()
{
    ui->stackedWidget->setCurrentWidget(m_wifiStaPage);
}

void WifiPage::SubPageInit()
{
    // 创建 wifi 连接子页面，并添加到 stackedWidget
    m_wifiConnPage = new WifiConnPage(this);
    ui->stackedWidget->addWidget(m_wifiConnPage);
    // 连接信号槽
    connect(m_wifiConnPage, &WifiConnPage::connectWifiRequested, this, &WifiPage::onConnectRequestFromSubPage);
    connect(m_wifiConnPage, &WifiConnPage::disconnectWifiRequested, this, &WifiPage::onDisconnectRequestFromSubPage);
    connect(m_wifiConnPage, &WifiConnPage::getWifiStatusRequested, this, &WifiPage::onGetStatusRequestFromSubPage);
    connect(this, &WifiPage::getWifiStatusResultToSubPage, m_wifiConnPage, &WifiConnPage::onGetWifiStatusResult);
    // 切换到 staPage 请求
    connect(m_wifiConnPage, &WifiConnPage::switchToStaPageRequested, this, &WifiPage::onSwitchToStaPageRequestFromConnSubPage);
    // 显示初始化
    m_wifiConnPage->Init();

    // 创建 wifi 连接状态子页面，并添加到 stackedWidget
    m_wifiStaPage = new WifiStaPage(this);
    ui->stackedWidget->addWidget(m_wifiStaPage);
    // 连接信号槽
    connect(m_wifiStaPage, &WifiStaPage::disconnectWifiRequested, this, &WifiPage::onDisconnectRequestFromSubPage);
    connect(m_wifiStaPage, &WifiStaPage::getWifiStatusRequested, this, &WifiPage::onGetStatusRequestFromSubPage);
    connect(this, &WifiPage::getWifiStatusResultToSubPage, m_wifiStaPage, &WifiStaPage::onGetWifiStatusResult);

    // 默认显示 wifi 连接子页面
    ui->stackedWidget->setCurrentWidget(m_wifiConnPage);
}
