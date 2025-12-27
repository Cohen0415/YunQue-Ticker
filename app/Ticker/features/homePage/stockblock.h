#ifndef STOCKBLOCK_H
#define STOCKBLOCK_H

#include <QWidget>
#include "features/homePage/stockinfo.h"

namespace Ui {
class StockBlock;
}

class StockBlock : public QWidget
{
    Q_OBJECT

public:

    explicit StockBlock(QWidget *parent = nullptr);
    ~StockBlock();

    void setStockInfo(const StockInfo &info); // 提供接口，外部设置内容

private:

    Ui::StockBlock *ui;
    StockInfo m_stockInfo;

    void updateUI(); // 渲染UI

};

#endif // STOCKBLOCK_H
