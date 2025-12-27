#include "stockblock.h"
#include "ui_stockblock.h"
#include "utils/log/logger.h"
#include "utils/qssload/qssloader.h"

StockBlock::StockBlock(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StockBlock)
{
    ui->setupUi(this);

    // 加载样式表
    QString qss = QssLoader::load(":/res/qss/pageQss/stockBlock.qss");
    if (!qss.isEmpty())
        this->setStyleSheet(qss);
}

StockBlock::~StockBlock()
{
    delete ui;
}

// 设置股票信息并更新 UI
void StockBlock::setStockInfo(const StockInfo &info)
{
    m_stockInfo = info;
    updateUI();
}

// 更新 UI 显示
void StockBlock::updateUI()
{
    ui->codeLabel->setText(m_stockInfo.code);   // 更新股票代码
    ui->nameLabel->setText(m_stockInfo.name);   // 更新股票名称

    ui->priceLabel->setText(QString::number(m_stockInfo.currentPrice, 'f', 2)); // 当前价格

    QString prefixSymbol = "";
    if (m_stockInfo.isRise)
    {
        prefixSymbol = "+";
    }
    else
    {
        prefixSymbol = "-";
    }
    ui->risePriceLabel->setText(QString("%1 %2").arg(prefixSymbol, QString::number(m_stockInfo.risePrice, 'f', 2)));
    ui->risePctLabel->setText(QString("%1 %2%").arg(prefixSymbol, QString::number(m_stockInfo.risePct, 'f', 2)));

    // 根据涨跌状态设置颜色
    if (m_stockInfo.isRise)
    {
        ui->priceLabel->setStyleSheet("color: #E33C64;");
        ui->risePriceLabel->setStyleSheet("color: #E33C64;");
        ui->risePctLabel->setStyleSheet("color: #E33C64;");

        // 上涨显示红色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/redCir.png"));
    }
    else
    {
        ui->priceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePriceLabel->setStyleSheet("color: #43CF7C;");
        ui->risePctLabel->setStyleSheet("color: #43CF7C;");

        // 上涨显示绿色 Icon
        ui->riseIconLabel->setPixmap(QPixmap(":/res/icon/pageIcon/homePage/greenCir.png"));
    }
}
