#include "homepage.h"
#include "ui_homepage.h"
#include "utils/log/logger.h"

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePage)
{
    ui->setupUi(this);
}

HomePage::~HomePage()
{
    delete ui;
}

// 页面进入回调
void HomePage::onPageEnter()
{
    LOG_DEBUG("HomePage entered.");
}

// 页面离开回调
void HomePage::onPageLeave()
{
    LOG_DEBUG("HomePage left.");
}
