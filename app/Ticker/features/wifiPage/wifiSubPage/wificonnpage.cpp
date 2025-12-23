#include "wificonnpage.h"
#include "ui_wificonnpage.h"

WifiConnPage::WifiConnPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiConnPage)
{
    ui->setupUi(this);

    // UI 初始化
    UIInit();
}

WifiConnPage::~WifiConnPage()
{
    delete ui;
}

void WifiConnPage::on_connButton_clicked()
{
    // 获取用户输入的 ssid 和 pwd
    QString ssid = ui->ssidLineEdit->text();
    QString password = ui->pwdLineEdit->text();

    // 2、发出连接请求信号
    // todo
}

void WifiConnPage::UIInit()
{
    // 错误提示语默认为空
    ui->inputHintLabel->setText("");

    // 用户输入框默认为空
    ui->ssidLineEdit->setText("");
    ui->pwdLineEdit->setText("");

    // 连接按钮默认不可使用
    ui->connButton->setEnabled(false);
}

void WifiConnPage::inputLineInspect()
{
    ui->connButton->setEnabled(false);

    // 检测 ssid 为空，则提示
    QString ssid = ui->ssidLineEdit->text();
    if (ssid.isEmpty())
    {
        ui->inputHintLabel->setText("Wi-Fi 名称不能为空！");
        return;
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
        return;
    }
    else
    {
        ui->inputHintLabel->setText("");
    }

    // 检测 pwd 小于 8 位，则提示
    if (pwd.length() < 8)
    {
        ui->inputHintLabel->setText("Wi-Fi 密码不能少于 8 位！");
        return;
    }
    else
    {
        ui->inputHintLabel->setText("");
    }

    ui->connButton->setEnabled(true);
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


