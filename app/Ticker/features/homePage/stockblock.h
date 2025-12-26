#ifndef STOCKBLOCK_H
#define STOCKBLOCK_H

#include <QWidget>

namespace Ui {
class StockBlock;
}

class StockBlock : public QWidget
{
    Q_OBJECT

public:
    explicit StockBlock(QWidget *parent = nullptr);
    ~StockBlock();

private:
    Ui::StockBlock *ui;
};

#endif // STOCKBLOCK_H
