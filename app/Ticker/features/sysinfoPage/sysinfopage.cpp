#include "sysinfopage.h"
#include "ui_sysinfopage.h"
#include "utils/log/logger.h"

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

// 页面进入回调
void SysinfoPage::onPageEnter()
{
    LOG_DEBUG("SysinfoPage entered.");
}

// 页面离开回调
void SysinfoPage::onPageLeave()
{
    LOG_DEBUG("SysinfoPage left.");
}
