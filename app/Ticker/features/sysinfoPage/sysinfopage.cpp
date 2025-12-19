#include "sysinfopage.h"
#include "ui_sysinfopage.h"

SysinfoPage::SysinfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SysinfoPage)
{
    ui->setupUi(this);
}

SysinfoPage::~SysinfoPage()
{
    delete ui;
}
