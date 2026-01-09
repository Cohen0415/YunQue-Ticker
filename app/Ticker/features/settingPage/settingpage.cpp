#include "settingpage.h"
#include "ui_settingpage.h"
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

static QString getSettingFilePath()
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(baseDir); // 确保路径存在
    return baseDir + "/" + SETTING_INFO_CONFIG_FILE_NAME;
}

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);
}

SettingPage::~SettingPage()
{
    delete ui;
}

// 页面初始化，给 widget.cpp 调用
void SettingPage::init()
{
    // 初始化 UI
    uiInit();

    // 如果本地配置文件不存在，则使用默认值
    QString path = getSettingFilePath();
    QFile file(path);
    if (!file.exists())
    {
        LOG_WARN("Setting file does not exist, using default settings.");
        ui->backlightHarSlider->setValue(80); // 默认背光 80
        ui->soundHarSlider->setValue(50);     // 默认音量 50
        return;
    }

    // 从本地加载设置
    loadSettingFromLocal();
    emit setBacklightRequested(ui->backlightHarSlider->value());
    emit setVolumeRequested(ui->soundHarSlider->value());
}

// UI 初始化
void SettingPage::uiInit()
{
    // 初始化背光滑动条的范围为 0～100
    ui->backlightHarSlider->setMinimum(BACKLIGHT_MIN_VALUE);
    ui->backlightHarSlider->setMaximum(BACKLIGHT_MAX_VALUE);

    // 初始化音量滑动条的范围为 0～100
    ui->soundHarSlider->setMinimum(VOLUME_MIN_VALUE);
    ui->soundHarSlider->setMaximum(VOLUME_MAX_VALUE);

    // 加载样式表
    QString qss = QssLoader::load(":/res/qss/pageQss/settingPage.qss");
    if (!qss.isEmpty())
        this->setStyleSheet(qss);
}

// 更新背光相关控件的 UI 显示
void SettingPage::updateBacklightUI(int value)
{

    ui->backlightValueLabel->setText(QString::number(value));
    ui->backlightHarSlider->setValue(value);
}

// 更新音量相关控件的 UI 显示
void SettingPage::updateVolumeUI(int value)
{
    // 如果音量为 0, 相关控件变成 disabled，复选框状态变成勾选
    if (!(ui->soundCheckBox->isChecked()))
    {
        ui->soundValueLabel->setText(QString::number(value));
        ui->soundHarSlider->setValue(value);

        if (value <= 0)
        {
            ui->soundHarSlider->setEnabled(false);
            ui->soundPreBtn->setEnabled(false);
            ui->soundNextBtn->setEnabled(false);
            ui->soundCheckBox->setChecked(true);
            emit PageMsgManager::getInstance()->volumeMuteStateChanged(true);
        }
        else
        {
            ui->soundHarSlider->setEnabled(true);
            ui->soundPreBtn->setEnabled(true);
            ui->soundNextBtn->setEnabled(true);
            ui->soundCheckBox->setChecked(false);
            emit PageMsgManager::getInstance()->volumeMuteStateChanged(false);
        }
    }
    // 如果静音复选框被勾选，且旧音量不为 0，说明用户已经通过复选框一键静音
    else if (ui->soundCheckBox->isChecked() && m_oldVolumeValue != 0 && value == 0)
    {
        ui->soundValueLabel->setText(QString::number(m_oldVolumeValue));
        ui->soundHarSlider->setValue(m_oldVolumeValue);

        ui->soundHarSlider->setEnabled(false);
        ui->soundPreBtn->setEnabled(false);
        ui->soundNextBtn->setEnabled(false);

        ui->soundCheckBox->setChecked(true);
        emit PageMsgManager::getInstance()->volumeMuteStateChanged(true);

        return; // 直接返回，不更新 m_oldVolumeValue
    }

    m_oldVolumeValue = value;
}

// 接收 presenter 发送的背光设置结果
void SettingPage::onBacklightSetResult(bool success, int value)
{
    if (success)
    {
        // 更新背光相关控件的 UI 显示
        updateBacklightUI(value);

        // 保存设置到本地
        saveSettingToLocal();
    }
}

// 接收 presenter 发送的背光获取结果
void SettingPage::onBacklightGetResult(bool success, int value)
{
    if (success)
    {
        // 更新背光相关控件的 UI 显示
        updateBacklightUI(value);

        // 保存设置到本地
        saveSettingToLocal();
    }
}

// 接收 presenter 发送的音量设置结果
void SettingPage::onVolumeSetResult(bool success, int value)
{
    if (success)
    {
        // 更新音量相关控件的 UI 显示
        updateVolumeUI(value);

        // 保存设置到本地
        saveSettingToLocal();
    }
}

// 接收 presenter 发送的音量获取结果
void SettingPage::onVolumeGetResult(bool success, int value)
{
    if (success)
    {
        // 更新音量相关控件的 UI 显示
        updateVolumeUI(value);

        // 保存设置到本地
        saveSettingToLocal();
    }
}

// 背光滑动条释放槽函数
void SettingPage::on_backlightHarSlider_sliderReleased()
{
    // 向 presenter 发送背光设置请求信号
    emit setBacklightRequested(ui->backlightHarSlider->value());
}

// 音量滑动条释放槽函数
void SettingPage::on_soundHarSlider_sliderReleased()
{
    // 向 presenter 发送音量设置请求信号
    emit setVolumeRequested(ui->soundHarSlider->value());
}

// 背光滑动条值变化槽函数
void SettingPage::on_backlightHarSlider_valueChanged(int value)
{
    // 实时更新背光值显示
    ui->backlightValueLabel->setText(QString::number(value));
}

// 音量滑动条值变化槽函数
void SettingPage::on_soundHarSlider_valueChanged(int value)
{
    // 实时更新音量值显示
    ui->soundValueLabel->setText(QString::number(value));
}

// 背光减按钮槽函数
void SettingPage::on_backlightPreBtn_clicked()
{
    // 背光滑动条减 1
    int currentValue = ui->backlightHarSlider->value();
    if (currentValue > ui->backlightHarSlider->minimum())
    {
        currentValue = currentValue - BACKLIGHT_STEP_VALUE;
        ui->backlightHarSlider->setValue(currentValue);
        ui->backlightValueLabel->setText(QString::number(currentValue));

        // 向 presenter 发送背光设置请求信号
        emit setBacklightRequested(currentValue);
    }
}

// 背光加按钮槽函数
void SettingPage::on_backlightNextBtn_clicked()
{
    // 背光滑动条加 1
    int currentValue = ui->backlightHarSlider->value();
    if (currentValue < ui->backlightHarSlider->maximum())
    {
        currentValue = currentValue + BACKLIGHT_STEP_VALUE;
        ui->backlightHarSlider->setValue(currentValue);
        ui->backlightValueLabel->setText(QString::number(currentValue));

        // 向 presenter 发送背光设置请求信号
        emit setBacklightRequested(currentValue);
    }
}

// 音量减按钮槽函数
void SettingPage::on_soundPreBtn_clicked()
{
    // 音量滑动条减 1
    int currentValue = ui->soundHarSlider->value();
    if (currentValue > ui->soundHarSlider->minimum())
    {
        currentValue = currentValue - VOLUME_STEP_VALUE;
        ui->soundHarSlider->setValue(currentValue);
        ui->soundValueLabel->setText(QString::number(currentValue));

        // 向 presenter 发送音量设置请求信号
        emit setVolumeRequested(currentValue);
    }
}

// 音量加按钮槽函数
void SettingPage::on_soundNextBtn_clicked()
{
    // 音量滑动条加 1
    int currentValue = ui->soundHarSlider->value();
    if (currentValue < ui->soundHarSlider->maximum())
    {
        currentValue = currentValue + VOLUME_STEP_VALUE;
        ui->soundHarSlider->setValue(currentValue);
        ui->soundValueLabel->setText(QString::number(currentValue));

        // 向 presenter 发送音量设置请求信号
        emit setVolumeRequested(currentValue);
    }
}

// 音量静音勾选框状态变化槽函数
void SettingPage::on_soundCheckBox_stateChanged(int arg1)
{
    // 静音勾选框
    // 静音
    if (arg1 == Qt::Checked)
    {
        // 用户想静音，向 presenter 发送音量设置请求信号，设置音量为 0
        emit setVolumeRequested(0);
    }
    else // 取消静音
    {
        // 用户想取消静音
        // 如果旧音量值有效，恢复旧音量值
        emit setVolumeRequested((m_oldVolumeValue > 0) ? m_oldVolumeValue : 50);
    }
}

// 页面进入回调
void SettingPage::onPageEnter()
{
    LOG_DEBUG("SettingPage entered.");
}

// 页面离开回调
void SettingPage::onPageLeave()
{
    LOG_DEBUG("SettingPage left.");
}

// 保存设置到本地
void SettingPage::saveSettingToLocal()
{
    QJsonObject settingObj;
    settingObj["backlight"] = ui->backlightHarSlider->value();
    settingObj["volume"] = ui->soundHarSlider->value();
    settingObj["volumeMuted"] = ui->soundCheckBox->isChecked();

    QJsonDocument doc(settingObj);
    QString path = getSettingFilePath();

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_WARN("Failed to open setting file: %s", path.toStdString().c_str());
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));

    if (!file.commit())   // 内部：fsync + rename（原子替换）
    {
        LOG_WARN("Failed to commit setting file: %s", path.toStdString().c_str());
        return;
    }

    LOG_INFO("Settings saved and synced, path: %s", path.toStdString().c_str());
}

// 从本地加载设置
void SettingPage::loadSettingFromLocal()
{
    QString path = getSettingFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    QJsonObject settingObj = doc.object();
    if (settingObj.contains("backlight") && settingObj["backlight"].isDouble())
    {
        int backlightValue = settingObj["backlight"].toInt();
        updateBacklightUI(backlightValue);
    }
    if (settingObj.contains("volume") && settingObj["volume"].isDouble())
    {
        int volumeValue = settingObj["volume"].toInt();
        updateVolumeUI(volumeValue);
    }
    if (settingObj.contains("volumeMuted") && settingObj["volumeMuted"].isBool())
    {
        bool volumeMuted = settingObj["volumeMuted"].toBool();
        ui->soundCheckBox->setChecked(volumeMuted);
    }

    LOG_INFO("Settings loaded from local, path: %s", path.toStdString().c_str());
}
