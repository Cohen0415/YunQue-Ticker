#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTimer>
#include <QSequentialAnimationGroup>

#include "features/pagelifecycleaware.h"
#include "features/homePage/stockportfolio.h"
#include "utils/stock/sinaquoteprovider.h"

/* 股票代码错误 */
#define STOCKCODE_ERR_LEN_INVALID       (-1)    // 股票代码长度无效
#define STOCKCODE_ERR_ILLEGAL           (-2)    // 股票代码不合法

/* 组合名词长度上限 */
#define PORTFOLIO_NAME_MAX_LEN          (7)     // 组合名称最大长度

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

    void init();                            // 页面初始化，给 widget.cpp 调用

protected:

    bool eventFilter(QObject *obj, QEvent *event) override;             // 事件过滤器重载，用于实现滚动拖拽

signals:

public slots:

private slots:

    void on_addNewStockBtn_clicked();       // 添加股票按钮槽函数
    void on_addNewPortfolioBtn_clicked();   // 添加组合按钮槽函数
    void on_portfolioComboBox_currentIndexChanged(int index);           // 组合下拉框索引变化槽函数
    void on_delPortfolioBtn_clicked();      // 删除组合按钮槽函数

    void on_quoteUpdateTimer_timeout();                                 // 行情定时请求更新槽函数
    void on_quoteProvider_updateQuotes(const QList<StockInfo> &infos);  // 接收最新行情槽函数
    void on_quoteProvider_updateQuote(const StockInfo &info);           // 接收单只股票行情槽函数
    void on_quoteProvider_error(const QString &stockCode, const QString &errReason); // 行情错误槽函数
    
    void on_delStockBlock_requested(const QString& code);               // 股票块删除请求槽函数

private:

    void uiInit();                                      // 初始化 UI 组件
    int isStockCode(const QString& rawInput, QString& outNormalizedCode);  // 验证股票代码合法性
    int getIndexOfPortfolio(const QString& name);       // 获取组合索引
    void updatePortfolioComboBox();                     // 更新组合下拉框显示

    void printAllPortfolioList();                       // 打印所有组合名称及其下的股票代码
    void printCurrentPortfolioStocks();                 // 打印当前组合名称及其下的股票列表

    void saveAllPortfoliosToLocal();                    // 保存所有组合到本地
    void loadAllPortfoliosFromLocal();                  // 从本地加载所有组合

    void refreshStockInfoDisplay();                     // 刷新当前组合下已有股票的信息显示
    void updateStockBlocks();                           // 更新当前组合下的股票块

    void installScrollDragFilters();                    // 初始化滚动

    void startBreathAnimation();                        // 启动呼吸灯动画
    void stopBreathAnimation();                         // 停止呼吸灯动画

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

    QSequentialAnimationGroup *m_breathAnimation = nullptr; // 呼吸灯动画
};

#endif // HOMEPAGE_H
