#ifndef STOCKBLOCK_H
#define STOCKBLOCK_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

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
    void breathOpacityOn(QWidget *label, QGraphicsOpacityEffect*& eff); // 呼吸灯效果
    void showDeleteBtnWithAnim();               // 删除按钮的缩放动画

    Ui::StockBlock *ui;
    StockInfo m_stockInfo;

    QTimer m_longPressTimer;                    // 长按定时器
    QPushButton *m_deleteBtn = nullptr;         // 删除按钮

    double m_lastPrice = 0, m_lastRise = 0, m_lastRisePct = 0;  // 上次的价格数据，用于判断是否变化
    QGraphicsOpacityEffect *m_priceOpacityEff = nullptr;        // 价格呼吸灯效果
    QGraphicsOpacityEffect *m_riseOpacityEff = nullptr;         // 涨跌呼吸灯效果
    QGraphicsOpacityEffect *m_pctOpacityEff = nullptr;          // 涨跌幅呼吸灯效果
    QGraphicsOpacityEffect *m_iconOpacityEff = nullptr;         // 涨跌图标呼吸灯效果

    QGraphicsOpacityEffect *m_delOpacityEff = nullptr;
    QPropertyAnimation *m_delOpacityAnim = nullptr;
    QPropertyAnimation *m_delScaleAnim = nullptr;
};

#endif // STOCKBLOCK_H
