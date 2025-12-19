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
