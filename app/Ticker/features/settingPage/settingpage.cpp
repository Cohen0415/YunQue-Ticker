#include "settingpage.h"
#include "ui_settingpage.h"
#include "utils/log/logger.h"
#include "features/pagemsgmanager.h"

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);

    UIinit();
}

SettingPage::~SettingPage()
{
    delete ui;
}

// UI 初始化
void SettingPage::UIinit()
{
    // 初始化背光滑动条的范围为 0～100
    ui->backlightHarSlider->setMinimum(0);
    ui->backlightHarSlider->setMaximum(100);

    // 初始化音量滑动条的范围为 0～100
    ui->soundHarSlider->setMinimum(0);
    ui->soundHarSlider->setMaximum(100);
}

// 接收 presenter 发送的背光设置结果
void SettingPage::onBacklightSetResult(bool success, int value)
{
    updateBacklightUI(success, value);
}

// 接收 presenter 发送的音量设置结果
void SettingPage::onVolumeSetResult(bool success, int value)
{
    updateVolumeUI(success, value);
}

// 更新背光 UI 显示
void SettingPage::updateBacklightUI(bool success, int value)
{
    if (success)
    {
        ui->backlightValueLabel->setText(QString::number(value));
        ui->backlightHarSlider->setValue(value);
    }
}

// 更新音量 UI 显示
void SettingPage::updateVolumeUI(bool success, int value)
{
    if (success)
    {
        ui->soundValueLabel->setText(QString::number(value));
        ui->soundHarSlider->setValue(value);
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
    ui->backlightValueLabel->setText(QString::number(value));
}

// 音量滑动条值变化槽函数
void SettingPage::on_soundHarSlider_valueChanged(int value)
{
    ui->soundValueLabel->setText(QString::number(value));
}

// 背光减按钮槽函数
void SettingPage::on_backlightPreBtn_clicked()
{
    // 背光滑动条减 1
    int currentValue = ui->backlightHarSlider->value();
    if (currentValue > ui->backlightHarSlider->minimum())
    {
        currentValue--;
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
        currentValue++;
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
        currentValue--;
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
        currentValue++;
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
    // 选中，直接禁用音量条和加减按钮，并发送 0 音量请求
    if (arg1 == Qt::Checked)
    {
        ui->soundHarSlider->setEnabled(false);
        ui->soundPreBtn->setEnabled(false);
        ui->soundNextBtn->setEnabled(false);
        // 向 presenter 发送音量设置请求信号，设置为 0
        emit setVolumeRequested(0);
        // 向 pageMsgManager 发送静音状态变化信号
        emit PageMsgManager::getInstance()->volumeMuteStateChanged(true);
    }
    else // 取消静音
    {
        ui->soundHarSlider->setEnabled(true);
        ui->soundPreBtn->setEnabled(true);
        ui->soundNextBtn->setEnabled(true);
        // 向 presenter 发送音量设置请求信号，设置为滑动条当前值
        emit setVolumeRequested(ui->soundHarSlider->value());
        // 向 pageMsgManager 发送静音状态变化信号
        emit PageMsgManager::getInstance()->volumeMuteStateChanged(false);
    }
}






