#include "wifistapage.h"
#include "ui_wifistapage.h"

WifiStaPage::WifiStaPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WifiStaPage)
{
    ui->setupUi(this);
}

WifiStaPage::~WifiStaPage()
{
    delete ui;
}

void WifiStaPage::onGetWifiStatusResult(bool success, bool connected, QString &ssid, QString &ip, QString &rssi)
{

}
