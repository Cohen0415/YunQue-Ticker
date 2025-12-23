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
