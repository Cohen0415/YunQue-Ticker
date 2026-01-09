#include "wificonnpage.h"
#include "ui_wificonnpage.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"
#include "features/pagemsgmanager.h"

#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

static QString getWifiInfoFilePath()
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDir); // 确保路径存在
    return baseDir + "/" + WIFI_INFO_CONFIG_FILE_NAME;
}

WifiConnPage::WifiConnPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiConnPageWidget)
    , m_statusTimer(new QTimer(this))
    , m_initTimer(new QTimer(this))
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

// 页面初始化, 给父页面调用
void WifiConnPage::init()
{
    // 连接定时器槽函数
    connect(m_statusTimer, &QTimer::timeout, this, &WifiConnPage::onStatusTimerTimeout);
    connect(m_initTimer, &QTimer::timeout, this, &WifiConnPage::onInitTimerTimeout);

    // UI 初始化
    uiInit();

    // 获取保存的 Wi-Fi 列表
    QList<QPair<QString, QString>> wifiList = loadWifiInfoFromLocal();
    if (!wifiList.isEmpty())
    {
        // 加载第一个 Wi-Fi 信息到输入框
        LOG_INFO("Load saved Wi-Fi config: SSID=%s, PWD=%s",
                  wifiList.first().first.toStdString().c_str(),
                  wifiList.first().second.toStdString().c_str());
        ui->ssidLineEdit->setText(wifiList.first().first);
        ui->pwdLineEdit->setText(wifiList.first().second);
    }
    else
    {
        LOG_WARN("No saved Wi-Fi config found.");
    }

    // app 启动后，轮询获取当前的 wifi 连接状态
    m_initTimer->start(m_checkInterval);
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

// 接收父页面发送的获取 wifi 状态结果
void WifiConnPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{
    if (success && connected)
    {
        // 连接成功，停止定时器
        if (m_statusTimer->isActive())
            m_statusTimer->stop();
        if (m_initTimer->isActive())
            m_initTimer->stop();

        // 向 PageMsgManager 发送已连接信号
        emit PageMsgManager::getInstance()->wifiStatusChanged(true);

        // 切换到sta页面
        emit switchToStaPageRequested();

        // 保存 Wi-Fi 凭据到本地
        saveWifiInfoToLocal(ssid, ui->pwdLineEdit->text());
        
        // 复位提示语
        ui->inputHintLabel->setText("");

        // 复位清除按钮的状态
        ui->ssidClearButton->setEnabled(true);
        ui->pwdClearButton->setEnabled(true);

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
    // 用户输入检测
    bool ret = inputLineInspect();
    if (!ret)
        return;

    // 获取用户输入的 ssid 和 pwd
    QString ssid = ui->ssidLineEdit->text();
    QString password = ui->pwdLineEdit->text();

    // 关闭初始化轮询定时器
    if (m_initTimer->isActive())
        m_initTimer->stop();

    // 禁用按钮
    ui->connButton->setEnabled(false);
    ui->connButton->setText("连 接 中...");

    // 禁用输入框文本清除按钮
    ui->ssidClearButton->setEnabled(false);
    ui->pwdClearButton->setEnabled(false);

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
void WifiConnPage::uiInit()
{
    // 加载样式表
    QString style = QssLoader::load(":/res/qss/pageQss/wifiConnPage.qss");
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
    ui->ssidLineEdit->setMaxLength(WIFI_SSID_MAX_LEN);
    ui->pwdLineEdit->setMaxLength(WIFI_PWD_MAX_LEN);

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
        
        // 复位清除按钮的状态
        ui->ssidClearButton->setEnabled(true);
        ui->pwdClearButton->setEnabled(true);

        // 复位连接按钮
        ui->connButton->setEnabled(true);
        ui->connButton->setText("连 接");
    }
}

// 初始化轮询定时器槽函数
void WifiConnPage::onInitTimerTimeout()
{
    emit getWifiStatusRequested();  // 轮询请求主页面
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

// 保存 Wi-Fi 凭据到配置文件
void WifiConnPage::saveWifiInfoToLocal(const QString &ssid, const QString &password)
{
    // 先检查配置文件中是否保存过该 ssid 和密码组合，若已保存则不重复保存
    QList<QPair<QString, QString>> wifiList = loadWifiInfoFromLocal();
    for (const auto &pair : wifiList)
    {
        if (pair.first == ssid && pair.second == password)
        {
            LOG_DEBUG("Wi-Fi config already saved, ssid: %s", ssid.toStdString().c_str());
            return;
        }
    }

    QJsonObject wifiObj;
    wifiObj["ssid"] = ssid;
    wifiObj["password"] = password;

    QJsonDocument doc(wifiObj);
    QString path = getWifiInfoFilePath();

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_WARN("Failed to open Wi-Fi config file: %s", path.toStdString().c_str());
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));

    if (!file.commit())   // 内部：fsync + rename（原子替换）
    {
        LOG_WARN("Failed to commit Wi-Fi config file: %s", path.toStdString().c_str());
        return;
    }

    LOG_INFO("Wi-Fi config saved and synced, path: %s", path.toStdString().c_str());
}

// 从配置文件加载已保存的 Wi-Fi 凭据列表
QList<QPair<QString, QString>> WifiConnPage::loadWifiInfoFromLocal()
{
    QList<QPair<QString, QString>> wifiList;
    QString path = getWifiInfoFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return wifiList;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
        return wifiList;

    QJsonObject wifiObj = doc.object();
    QString ssid = wifiObj.value("ssid").toString();
    QString password = wifiObj.value("password").toString();
    if (!ssid.isEmpty())
    {
        wifiList.append(qMakePair(ssid, password));
    }   

    return wifiList;
}