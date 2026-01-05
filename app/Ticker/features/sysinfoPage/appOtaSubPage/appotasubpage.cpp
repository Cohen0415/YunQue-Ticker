#include "appotasubpage.h"
#include "ui_appotasubpage.h"

AppOtaSubPage::AppOtaSubPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppOtaSubPage)
{
    ui->setupUi(this);
}

AppOtaSubPage::~AppOtaSubPage()
{
    delete ui;
}
