#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTimer>

#include "features/pagelifecycleaware.h"
#include "features/homePage/stockportfolio.h"
#include "utils/stock/sinaquoteprovider.h"

namespace Ui {
class HomePage;
}

class HomePage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;
    void onPageLeave() override;

    void init();     // 页面初始化，给 widget.cpp 调用

protected:

    // 事件过滤器重载，用于实现滚动拖拽
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:

public slots:

private slots:

    void on_addNewStockBtn_clicked();       // 添加股票按钮槽函数

    void on_addNewPortfolioBtn_clicked();

    void on_portfolioComboBox_currentIndexChanged(int index);

    void on_delPortfolioBtn_clicked();

    void on_quoteProvider_updateQuotes(const QList<StockInfo> &infos);  // 行情更新槽函数
    void on_quoteProvider_error(const QString &stockCode, const QString &errReason); // 行情错误槽函数
    void on_quoteUpdateTimer_timeout();                                 // 行情定时更新槽函数
    
private:

    void uiInit();   // 初始化 UI 组件
    int isStockCode(const QString& rawInput, QString& outNormalizedCode);  // 验证股票代码合法性
    int getIndexOfPortfolio(const QString& name);       // 获取组合索引，找不到返回 -1
    void updatePortfolioComboBox();                     // 更新组合下拉框显示

    void printAllPortfolioList();                       // 打印所有组合列表，调试用
    void printCurrentPortfolioStocks();                 // 打印当前组合的股票列表，调试用

    void saveAllPortfoliosToLocal();                    // 保存所有组合到本地
    void loadAllPortfoliosFromLocal();                  // 从本地加载所有组合

    void refreshStockInfoDisplay();                     // 刷新股票信息显示
    void updateStockBlocks();                           // 更新股票块

    void installScrollDragFilters();                    // 初始化滚动

    Ui::HomePage *ui;
    QHBoxLayout *stockBlocksLayout;                     // 股票块水平布局

    QList<StockPortfolio*> m_portfolioList;             // 所有组合
    StockPortfolio* m_currentPortfolio;                 // 当前选中

    QScrollBar *m_horizontalScrollBar = nullptr;
    bool m_isHDragging = false;
    QPoint m_hDragStartPos;
    bool m_isHContentDragging = false;
    int m_hDragThreshold = 5;

    SinaQuoteProvider *m_quoteProvider;                  // 新浪行情提供者
    QTimer *m_quoteUpdateTimer;                          // 定时更新行情 Timer
};

#endif // HOMEPAGE_H
