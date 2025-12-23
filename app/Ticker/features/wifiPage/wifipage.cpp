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

}

// 接收 presenter 发送的 Wi-Fi 断开连接结果
void WifiPage::onDisconnectWifiResult(bool success)
{

}

// 接收 presenter 发送的获取 Wi-Fi 状态结果
void WifiPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{

}

void WifiPage::SubPageInit()
{
    // 创建 wifi 连接子页面，并添加到 stackedWidget
    m_wifiConnPage = new WifiConnPage(this);
    ui->stackedWidget->addWidget(m_wifiConnPage);

    // 创建 wifi 连接状态子页面，并添加到 stackedWidget
    m_wifiStaPage = new WifiStaPage(this);
    ui->stackedWidget->addWidget(m_wifiStaPage);

    // 默认显示 wifi 连接子页面
    ui->stackedWidget->setCurrentWidget(m_wifiConnPage);
}
