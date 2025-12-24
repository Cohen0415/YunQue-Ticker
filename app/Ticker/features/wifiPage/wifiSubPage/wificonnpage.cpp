#include "wificonnpage.h"
#include "ui_wificonnpage.h"
#include "utils/log/logger.h"
#include "features/pagemsgmanager.h"
#include <QFile>

WifiConnPage::WifiConnPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiConnPageWidget)
    , m_statusTimer(new QTimer(this))
{
    ui->setupUi(this);

    m_checkInterval = CHECK_WIFI_CONN_INTERVAL_MS;
    m_timeoutMs = WIFI_CONN_TIMEOUT_MS;
}

WifiConnPage::~WifiConnPage()
{
    if (m_statusTimer && m_statusTimer->isActive())
        m_statusTimer->stop();
    delete ui;
}

void WifiConnPage::Init()
{
    connect(m_statusTimer, &QTimer::timeout, this, &WifiConnPage::onStatusTimerTimeout);

    // UI 初始化
    UIInit();

    // 初始化时，请求一次当前的 wifi 连接状态
    emit getWifiStatusRequested();
}

// 页面进入回调
void WifiConnPage::onPageEnter()
{
    LOG_DEBUG("WifiConnPage entered.");

    // 复位提示语
    ui->inputHintLabel->setText("");
}

// 页面离开回调
void WifiConnPage::onPageLeave()
{
    LOG_DEBUG("WifiConnPage left.");
}

void WifiConnPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    if (success && connected)
    {
        // 连接成功，停止定时器
        if (m_statusTimer->isActive())
            m_statusTimer->stop();

        // 向 PageMsgManager 发送已连接信号
        emit PageMsgManager::getInstance()->wifiStatusChanged(true);

        // 切换到sta页面
        emit switchToStaPageRequested();

        // 复位提示语
        ui->inputHintLabel->setText("");

        // 复位用户输入框
        ui->ssidLineEdit->setEnabled(true);
        ui->pwdLineEdit->setEnabled(true);

        // 复位按钮
        ui->connButton->setText("连 接");
        ui->connButton->setEnabled(true);
    }
    else if (success && !connected)
    {
        emit PageMsgManager::getInstance()->wifiStatusChanged(false);
    }
    else
    {
        if (m_statusTimer->isActive())
            m_statusTimer->stop();
        emit PageMsgManager::getInstance()->wifiStatusChanged(false);
    }
}

// 连接按钮槽函数
void WifiConnPage::on_connButton_clicked()
{
    bool ret = inputLineInspect();
    if (!ret)
        return;

    // 获取用户输入的 ssid 和 pwd
    QString ssid = ui->ssidLineEdit->text();
    QString password = ui->pwdLineEdit->text();

    // 禁用按钮
    ui->connButton->setEnabled(false);
    ui->connButton->setText("连 接 中...");

    // 禁用用户输入框
    ui->ssidLineEdit->setEnabled(false);
    ui->pwdLineEdit->setEnabled(false);

    // 清空提示语
    ui->inputHintLabel->setText("");

    // 发出连接请求信号
    emit connectWifiRequested(ssid, password);

    // 启动超时计时与轮询
    m_elapsed = 0;
    if (!m_statusTimer->isActive())
        m_statusTimer->start(m_checkInterval);
}

// UI 初始化
void WifiConnPage::UIInit()
{
    // 加载样式表
    QString style = LoadQssStyle(":/res/qss/pageQss/wifiConnPage.qss");
    if (!style.isEmpty())
    {
        this->setStyleSheet(style);
    }

    // 复位清除按钮的文本
    ui->ssidClearButton->setText("");
    ui->pwdClearButton->setText("");

    // 错误提示语默认为空
    ui->inputHintLabel->setText("");

    // 用户输入框默认为空
    ui->ssidLineEdit->setText("");
    ui->pwdLineEdit->setText("");

    // 提示语默认为空
    ui->inputHintLabel->setText("");

    // 复位按钮提示语
    ui->connButton->setText("连 接");
    ui->connButton->setFocusPolicy(Qt::NoFocus);

    //
    ui->ssidClearButton->setFocusPolicy(Qt::NoFocus);
    ui->pwdClearButton->setFocusPolicy(Qt::NoFocus);
}

// wifi 信息输入检测
bool WifiConnPage::inputLineInspect()
{
    bool ret = false;

    // 检测 ssid 为空，则提示
    QString ssid = ui->ssidLineEdit->text();
    if (ssid.isEmpty())
    {
        ui->inputHintLabel->setText("Wi-Fi 名称不能为空！");
        return ret;
    }
    else
    {
        ui->inputHintLabel->setText("");
    }

    // 检测 pwd 为空，则提示
    QString pwd = ui->pwdLineEdit->text();
    if (pwd.isEmpty())
    {
        ui->inputHintLabel->setText("Wi-Fi 密码不能为空！");
        return ret;
    }
    else
    {
        ui->inputHintLabel->setText("");
    }

    // 检测 pwd 小于 8 位，则提示
    if (pwd.length() < 8)
    {
        ui->inputHintLabel->setText("Wi-Fi 密码不能少于 8 位！");
        return ret;
    }
    else
    {
        ui->inputHintLabel->setText("");
    }

    return true;
}

QString WifiConnPage::LoadQssStyle(const QString &path)
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

// PWD 输入框文本变化槽函数
void WifiConnPage::on_ssidLineEdit_textChanged(const QString &arg1)
{
    inputLineInspect();
}

// SSID 输入框文本变化槽函数
void WifiConnPage::on_pwdLineEdit_textChanged(const QString &arg1)
{
    inputLineInspect();
}

// 定时器槽函数，定时请求 wifi 连接状态
void WifiConnPage::onStatusTimerTimeout()
{
    m_elapsed += m_checkInterval;

    emit getWifiStatusRequested();  // 轮询请求主页面

    if (m_elapsed >= m_timeoutMs)   // 连接超时
    {
        // 发送断开连接请求
        emit disconnectWifiRequested();

        m_statusTimer->stop();
        ui->inputHintLabel->setText("Wi-Fi 连接超时，请检查ssid或密码！");

        // 复位用户输入框
        ui->ssidLineEdit->setEnabled(true);
        ui->pwdLineEdit->setEnabled(true);

        // 复位连接按钮
        ui->connButton->setEnabled(true);
        ui->connButton->setText("连 接");
    }
}

// SSID 清除按钮槽函数
void WifiConnPage::on_ssidClearButton_clicked()
{
    ui->ssidLineEdit->clear();
}

// PWD 清除按钮槽函数
void WifiConnPage::on_pwdClearButton_clicked()
{
    ui->pwdLineEdit->clear();
}

