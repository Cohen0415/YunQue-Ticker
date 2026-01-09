#include "wifipage.h"
#include "ui_wifipage.h"
#include "utils/log/logger.h"

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

// 页面初始化，给 widget.cpp 调用
void WifiPage::init()
{
    // 子页面初始化
    subPageInit();
}

// 接收 presenter 发送的 Wi-Fi 连接结果
void WifiPage::onConnectWifiResult(bool success)
{
    // not todo
}

// 接收 presenter 发送的 Wi-Fi 断开连接结果
void WifiPage::onDisconnectWifiResult(bool success)
{
    emit getWifiDisconnectResultToStaSubPage(success);
}

// 接收 presenter 发送的获取 Wi-Fi 状态结果
void WifiPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    // 将结果发送给子页面
    if (ui->stackedWidget->currentWidget() == m_wifiConnPage)       // 如果当前是 connPage
        emit getWifiStatusResultToConnSubPage(success, connected, ssid, ip, rssi);
    else if (ui->stackedWidget->currentWidget() == m_wifiStaPage)
        emit getWifiStatusResultToStaSubPage(success, connected, ssid, ip, rssi);
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
    // 切换到 staPage
    switchToPage(m_wifiStaPage);
}

// 接收 staPage 的切换到 connPage 的请求
void WifiPage::onSwitchToConnPageRequestFromStaSubPage()
{
    // 切换到 connPage
    switchToPage(m_wifiConnPage);
}

// 子页面初始化
void WifiPage::subPageInit()
{
    // 创建 wifi 连接子页面，并添加到 stackedWidget
    m_wifiConnPage = new WifiConnPage(this);
    ui->stackedWidget->addWidget(m_wifiConnPage);
    // 连接信号槽
    connect(m_wifiConnPage, &WifiConnPage::connectWifiRequested, this, &WifiPage::onConnectRequestFromSubPage);
    connect(m_wifiConnPage, &WifiConnPage::disconnectWifiRequested, this, &WifiPage::onDisconnectRequestFromSubPage);
    connect(m_wifiConnPage, &WifiConnPage::getWifiStatusRequested, this, &WifiPage::onGetStatusRequestFromSubPage);
    connect(this, &WifiPage::getWifiStatusResultToConnSubPage, m_wifiConnPage, &WifiConnPage::onGetWifiStatusResult);
    // 切换到 staPage 请求
    connect(m_wifiConnPage, &WifiConnPage::switchToStaPageRequested, this, &WifiPage::onSwitchToStaPageRequestFromConnSubPage);
    // 显示初始化
    m_wifiConnPage->init();

    // 创建 wifi 连接状态子页面，并添加到 stackedWidget
    m_wifiStaPage = new WifiStaPage(this);
    ui->stackedWidget->addWidget(m_wifiStaPage);
    // 连接信号槽
    connect(m_wifiStaPage, &WifiStaPage::disconnectWifiRequested, this, &WifiPage::onDisconnectRequestFromSubPage);
    connect(m_wifiStaPage, &WifiStaPage::getWifiStatusRequested, this, &WifiPage::onGetStatusRequestFromSubPage);
    connect(this, &WifiPage::getWifiStatusResultToStaSubPage, m_wifiStaPage, &WifiStaPage::onGetWifiStatusResult);
    connect(this, &WifiPage::getWifiDisconnectResultToStaSubPage, m_wifiStaPage, &WifiStaPage::onGetWifiDisconnectResult);
    // 切换到 connPage 请求
    connect(m_wifiStaPage, &WifiStaPage::switchToConnPageRequested, this, &WifiPage::onSwitchToConnPageRequestFromStaSubPage);
    // 显示初始化
    m_wifiStaPage->init();

    // 默认显示 wifi 连接子页面
    ui->stackedWidget->setCurrentWidget(m_wifiConnPage);
}

// 切换到指定页面
void WifiPage::switchToPage(QWidget *target)
{
    // 如果目标页等于当前页，直接返回
    if (ui->stackedWidget->currentWidget() == target)
        return;

    // 先通知上一个页面
    if (m_lastPageWidget)
    {
        auto lastAble = dynamic_cast<PageLifecycleAware *>(m_lastPageWidget);
        if (lastAble)
            lastAble->onPageLeave();
    }

    // 切换页面
    ui->stackedWidget->setCurrentWidget(target);

    // 再通知新页面
    auto newAble = dynamic_cast<PageLifecycleAware *>(target);
    if (newAble)
        newAble->onPageEnter();

    // 更新last
    m_lastPageWidget = target;
}

// 页面进入回调
void WifiPage::onPageEnter()
{
    LOG_DEBUG("WifiPage entered.");

    // 显示调用子页的进入函数
    if (ui->stackedWidget->currentWidget() == m_wifiStaPage)
    {
        m_wifiStaPage->onPageEnter();
    }
    else if (ui->stackedWidget->currentWidget() == m_wifiConnPage)
    {
        m_wifiConnPage->onPageEnter();
    }
}

// 页面离开回调
void WifiPage::onPageLeave()
{
    LOG_DEBUG("WifiPage left.");

    // 显示调用子页的离开函数
    if (ui->stackedWidget->currentWidget() == m_wifiStaPage)
    {
        m_wifiStaPage->onPageLeave();
    }
    else if (ui->stackedWidget->currentWidget() == m_wifiConnPage)
    {
        m_wifiConnPage->onPageLeave();
    }
}
