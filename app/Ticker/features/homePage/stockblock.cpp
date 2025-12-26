#include "stockblock.h"
#include "ui_stockblock.h"

StockBlock::StockBlock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StockBlock)
{
    ui->setupUi(this);
}

StockBlock::~StockBlock()
{
    delete ui;
}
