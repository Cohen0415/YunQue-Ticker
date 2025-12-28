#ifndef STOCKBLOCK_H
#define STOCKBLOCK_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include "features/homePage/stockinfo.h"

#define LONG_PRESS_THRESHOLD_MS    (800)   // 长按阈值，单位毫秒

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

signals:

    void deleteRequested(const QString& code); // 通知主页要删除自己

protected:

    void mousePressEvent(QMouseEvent *event) override;      // 鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *event) override;    // 鼠标释放事件
    void leaveEvent(QEvent *event) override;                // 鼠标离开事件

private slots:

    void onLongPress();                         // 长按处理槽函数
    void onDelBtnPress();                       // 删除按钮点击槽函数

private:

    void updateUI();                            // 渲染UI
    void ensureDeleteBtn();                     // 创建删除按钮

    Ui::StockBlock *ui;
    StockInfo m_stockInfo;

    QTimer m_longPressTimer;                    // 长按定时器
    QPushButton *m_deleteBtn = nullptr;         // 删除按钮
};

#endif // STOCKBLOCK_H
