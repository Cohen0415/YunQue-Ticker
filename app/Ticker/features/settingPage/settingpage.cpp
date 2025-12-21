#include "settingpage.h"
#include "ui_settingpage.h"
#include "utils/log/logger.h"

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

void SettingPage::on_backlightHarSlider_sliderReleased()
{
    // 向 presenter 发送背光设置请求信号
    emit setBacklightRequested(ui->backlightHarSlider->value());
}

void SettingPage::on_soundHarSlider_sliderReleased()
{
    // 向 presenter 发送音量设置请求信号
    emit setVolumeRequested(ui->soundHarSlider->value());
}
